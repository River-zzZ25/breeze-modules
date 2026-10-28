#pragma once

#include <etl/circular_buffer.h>
#include <etl/delegate.h>
#include <etl/span.h>

namespace breeze::referee
{

struct RawPackage
{
	uint16_t CmdId;
	etl::span<const uint8_t> Payload;
};

/**
 * @brief 流式解析器, 带状态, 鲁棒地处理串口流
 * @tparam TOnNext 解析完成后的静态回调
 * @tparam TBufferSize 缓冲区大小
 */
class StreamParser final
{
public:
	using Observer = etl::delegate<void(RawPackage)>;
	using RingBuffer = etl::icircular_buffer<uint8_t>;
	using TmpBuffer = etl::span<uint8_t>;

	/**
	 * @brief Construct a new Stream Parser object
	 *
	 * @param ringBuffer 用于临时接收数据的环形缓冲区, 需要大于单包大小
	 * @param tmpBuffer 用于临时处理数据的线性缓冲区(数组), 需要大于或等于单包大小
	 * @param onNext 下游通知委托
	 */
	StreamParser(RingBuffer& ringBuffer, TmpBuffer tmpBuffer, Observer onNext)
	    : _ringBuffer(ringBuffer),
	      _tmpBuffer(tmpBuffer),
	      _onNext(onNext)
	{}

	/**
	 * @brief 同步流式读入数据, 触发解析
	 *
	 * @param data
	 */
	void Feed(etl::span<const uint8_t> data);

private:
	RingBuffer& _ringBuffer;
	TmpBuffer _tmpBuffer;
	Observer _onNext;
};

} // namespace breeze::referee
