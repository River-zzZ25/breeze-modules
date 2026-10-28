#include "protocol/parser.h"
#include "protocol/referee.h"
#include "stats.h"
#include <cassert>
#include <cstdint>
#include <etl/algorithm.h>
#include <etl/byte_stream.h>
#include <etl/circular_buffer.h>
#include <etl/endianness.h>
#include <etl/iterator.h>
#include <robotpilots/algorithm/crc.hpp>
namespace breeze::referee
{

// 移植到具体CRC实现 (其实是命名太长了)
static inline auto CalcCrc8(etl::span<const uint8_t> data) { return robotpilots::algorithm::Crc8Calculate(data); }
static inline auto CalcCrc16(etl::span<const uint8_t> data) { return robotpilots::algorithm::Crc16Calculate(data); }

constexpr auto HEADER_SIZE = sizeof(protocol::FrameHeader);
constexpr auto TAIL_SIZE = sizeof(uint16_t);
constexpr auto CMD_SIZE = sizeof(uint16_t);
using Iterator = StreamParser::RingBuffer::iterator;

/**
 * @brief 搜索有效帧并通知
 *
 * @param it_next 当前迭代器, 表示处理进度, 且影响外部循环
 * @param it_end 迭代器边界
 * @param on_next 下游委托
 * @return iterator 新的迭代器, 将会丢弃迭代器前的所有内容
 */
static Iterator SearchAndEmit(
    Iterator& itNext,
    const Iterator itEnd,
    StreamParser::Observer onNext,
    StreamParser::TmpBuffer buffer
)
{
	const auto itFirst = itNext;
	const auto sof = etl::find(itNext, itEnd, protocol::SOF);

	// 没找到sof, 全部数据都没用, 直接丢掉并退出.
	if (sof == itEnd)
	{
		internal::stats::Record(internal::stats::Event::ParserDiscardBytes, static_cast<uint32_t>(itEnd - itFirst));
		itNext = itEnd;
		return itEnd;
	}

	const size_t avail = static_cast<size_t>(itEnd - sof);
	internal::stats::Record(internal::stats::Event::ParserSofFound);

	// 数据不够, 等待下次解析
	if (avail < HEADER_SIZE)
	{
		itNext = itEnd;
		return itFirst;
	}

	// 把header 放进连续的buffer里便于解析
	etl::copy(sof, sof + HEADER_SIZE, buffer.begin());

	etl::byte_stream_reader headerReader{buffer.data(), HEADER_SIZE, etl::endian::little};

	const protocol::FrameHeader header{
	    .Sof = *headerReader.read<decltype(header.Sof)>(),
	    .DataLength = *headerReader.read<decltype(header.DataLength)>(), // 这里optional都可以直接解, 因为上面检查过长度
	    .Seq = *headerReader.read<decltype(header.Seq)>(),
	    .Crc8 = *headerReader.read<decltype(header.Crc8)>()
	};

	// 帧头有问题, 从下一个字节继续.
	if (const auto crc = CalcCrc8({buffer.begin(), buffer.begin() + HEADER_SIZE - 1}); crc != header.Crc8)
	{
		internal::stats::Record(internal::stats::Event::ParserHeaderCrcError);
		internal::stats::Record(internal::stats::Event::ParserDiscardBytes);
		itNext = sof + 1;
		return itNext;
	}

	// 帧头没毛病, 数据不够, 丢掉sof前内容, 并结束解析.
	if (avail < HEADER_SIZE + CMD_SIZE + header.DataLength + TAIL_SIZE)
	{
		itNext = itEnd;
		return sof;
	}

	// 拷贝全包到线性数组以检查crc
	const size_t maxData = buffer.size() - HEADER_SIZE - CMD_SIZE - TAIL_SIZE;
	if (header.DataLength > maxData)
	{
		internal::stats::Record(internal::stats::Event::ParserLengthError);
		internal::stats::Record(internal::stats::Event::ParserDiscardBytes);
		itNext = sof + 1;
		return itNext;
	}

	etl::copy(
	    sof + HEADER_SIZE, sof + HEADER_SIZE + CMD_SIZE + header.DataLength + TAIL_SIZE, buffer.begin() + HEADER_SIZE
	);

	etl::byte_stream_reader tailReader{
	    buffer.data() + HEADER_SIZE + CMD_SIZE + header.DataLength, TAIL_SIZE, etl::endian::little
	};
	const auto tailCrc = *tailReader.read<uint16_t>();

	// 尾部crc不对, 直接丢掉解析过的东西
	if (const auto crc = CalcCrc16({buffer.begin(), buffer.begin() + HEADER_SIZE + CMD_SIZE + header.DataLength});
	    crc != tailCrc)
	{
		internal::stats::Record(internal::stats::Event::ParserFrameCrcError);
		internal::stats::Record(
		    internal::stats::Event::ParserDiscardBytes,
		    static_cast<uint32_t>(HEADER_SIZE + CMD_SIZE + header.DataLength + TAIL_SIZE)
		);
		itNext = sof + HEADER_SIZE + CMD_SIZE + header.DataLength + TAIL_SIZE;
		return itNext;
	}

	// 尾部crc也对, 数据有效, 允许起飞, 并且不再解析这些内容.

	etl::byte_stream_reader cmdReader{buffer.data() + HEADER_SIZE, CMD_SIZE, etl::endian::little};
	const auto cmd = *cmdReader.read<uint16_t>();

	const RawPackage pkg{
	    .CmdId = cmd,
	    .Payload = {
	        buffer.begin() + HEADER_SIZE + CMD_SIZE, buffer.begin() + HEADER_SIZE + CMD_SIZE + header.DataLength
	    }
	};

	onNext(pkg);
	internal::stats::Record(internal::stats::Event::ParserFramesOk);
	internal::stats::Record(internal::stats::Event::RawPackagesEmitted);

	itNext = sof + HEADER_SIZE + CMD_SIZE + header.DataLength + TAIL_SIZE;
	return itNext;
}

void StreamParser::Feed(etl::span<const uint8_t> data)
{
	if (_ringBuffer.size() < data.size())
	{
		_ringBuffer.pop(data.size() - _ringBuffer.size());
	}

	_ringBuffer.push(data.begin(), data.end());

	const auto itBegin = _ringBuffer.begin();
	const auto itEnd = _ringBuffer.end();

	auto itNext = _ringBuffer.begin();
	auto itRemain = _ringBuffer.begin();

	while (itNext != itEnd)
	{
		itRemain = SearchAndEmit(itNext, itEnd, _onNext, _tmpBuffer);
	}
	const auto dropCount = static_cast<size_t>(itRemain - itBegin);
	_ringBuffer.pop(dropCount);
}

} // namespace breeze::referee
