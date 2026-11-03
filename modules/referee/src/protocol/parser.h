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
 * @brief 流式帧解析算法. 缓冲区和观察者由调用方提供.
 */
struct StreamParser final
{
	using Observer = etl::delegate<void(RawPackage)>;
	using RingBuffer = etl::icircular_buffer<uint8_t>;
	using TmpBuffer = etl::span<uint8_t>;

	/**
	 * @brief 同步读入数据并解析完整帧.
	 * @param ringBuffer 跨调用保留未完成数据的环形缓冲区
	 * @param tmpBuffer CRC 校验和解码使用的线性缓冲区
	 * @param data 新输入的数据块
	 * @param onNext 完整帧观察者, 仅在本次调用期间使用
	 */
	static void Feed(RingBuffer& ringBuffer, TmpBuffer tmpBuffer, etl::span<const uint8_t> data, Observer onNext);
};

} // namespace breeze::referee
