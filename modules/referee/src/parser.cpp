#include "protocol/parser.h"
#include "protocol/referee.h"
#include <assert.h>
#include <etl/algorithm.h>
#include <etl/circular_buffer.h>
#include <etl/endianness.h>
#include <etl/iterator.h>
#include <etl/byte_stream.h>
#include <cstdint>
#include <robotpilots/algorithm/algo_crc.hpp>
namespace breeze::referee
{

// 移植到具体CRC实现 (其实是命名太长了)
static inline auto calc_crc8(etl::span<const uint8_t> data)
{ return robotpilots::algorithm::Crc8Calculate(data); }
static inline auto calc_crc16(etl::span<const uint8_t> data)
{ return robotpilots::algorithm::Crc16Calculate(data); }

constexpr auto header_size = sizeof(protocol::frame_header);
constexpr auto tail_size = sizeof(uint16_t);
constexpr auto cmd_size = sizeof(uint16_t);
using iterator = StreamParser::RingBuffer::iterator;

/**
 * @brief 搜索有效帧并通知
 * 
 * @param it_next 当前迭代器, 表示处理进度, 且影响外部循环
 * @param it_end 迭代器边界
 * @param on_next 下游委托
 * @return iterator 新的迭代器, 将会丢弃迭代器前的所有内容
 */
static iterator search_and_emit(
    iterator& it_next,
    const iterator it_end,
    StreamParser::Observer on_next,
    StreamParser::TmpBuffer buffer
)
{
    const auto it_first = it_next;
    const auto sof = etl::find(
        it_next,
        it_end,
        protocol::sof
    );

    // 没找到sof, 全部数据都没用, 直接丢掉并退出.
    if(sof == it_end)
    {
        it_next = it_end;
        return it_end;
    }

    const size_t avail = static_cast<size_t>(it_end - sof);

    // 数据不够, 等待下次解析
    if(avail < header_size)
    {
        it_next = it_end;
        return it_first;
    }

    // 把header 放进连续的buffer里便于解析
    etl::copy(sof, sof + header_size, buffer.begin());

    etl::byte_stream_reader header_reader {
        buffer.data(),
        header_size,
        etl::endian::little
    };

    const protocol::frame_header header {
        .sof = *header_reader.read<decltype(header.sof)>(),
        .data_length = *header_reader.read<decltype(header.data_length)>(), // 这里optional都可以直接解, 因为上面检查过长度
        .seq = *header_reader.read<decltype(header.seq)>(),
        .crc8 = *header_reader.read<decltype(header.crc8)>()
    };

    // 帧头有问题, 从下一个字节继续.
    if(const auto crc = calc_crc8({buffer.begin(), buffer.begin() + header_size - 1});
            crc != header.crc8)
    {
        it_next = sof + 1;
        return it_next;
    }

    // 帧头没毛病, 数据不够, 丢掉sof前内容, 并结束解析.
    if(avail < header_size + cmd_size + header.data_length + tail_size)
    {
        it_next = it_end;
        return sof;
    }

    // 拷贝全包到线性数组以检查crc
    const size_t max_data = buffer.size() - header_size - cmd_size - tail_size;
    if (header.data_length > max_data) {
        it_next = sof + 1;
        return it_next;
    }

    etl::copy(sof + header_size, sof + header_size + cmd_size + header.data_length + tail_size, buffer.begin() + header_size);

    etl::byte_stream_reader tail_reader {
        buffer.data() + header_size + cmd_size + header.data_length,
        tail_size,
        etl::endian::little
    };
    const auto tail_crc = *tail_reader.read<uint16_t>();

    // 尾部crc不对, 直接丢掉解析过的东西
    if(const auto crc = calc_crc16({buffer.begin(), buffer.begin() + header_size + cmd_size + header.data_length});
        crc != tail_crc)
    {
        it_next = sof + header_size + cmd_size + header.data_length + tail_size;
        return it_next;
    }
    
    // 尾部crc也对, 数据有效, 允许起飞, 并且不再解析这些内容.

    etl::byte_stream_reader cmd_reader {
        buffer.data() + header_size,
        cmd_size,
        etl::endian::little
    };
    const auto cmd = *cmd_reader.read<uint16_t>();

    const raw_package pkg {
        .cmd_id = cmd,
        .payload =
        {
            buffer.begin() + header_size + cmd_size,
            buffer.begin() + header_size + cmd_size + header.data_length
        }
    };

    on_next(pkg);

    it_next = sof + header_size + cmd_size + header.data_length + tail_size;
    return it_next;
}

void StreamParser::feed(etl::span<const uint8_t> data)
{
    if(_ring_buffer.size() < data.size())
        _ring_buffer.pop(data.size() - _ring_buffer.size());

    _ring_buffer.push(data.begin(), data.end());

    const auto it_begin = _ring_buffer.begin();
    const auto it_end = _ring_buffer.end();

    auto it_next = _ring_buffer.begin();
    auto it_remain = _ring_buffer.begin();

    while(it_next != it_end)
    {
        it_remain = search_and_emit(it_next, it_end, _on_next, _tmp_buffer);
    }
    const auto drop_count = static_cast<size_t>(it_remain - it_begin);
    _ring_buffer.pop(drop_count);
}

}
