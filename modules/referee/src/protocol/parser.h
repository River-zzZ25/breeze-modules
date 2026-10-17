#pragma once

#include <etl/span.h>
#include <etl/circular_buffer.h>
#include <etl/delegate.h>

namespace breeze::referee
{

struct raw_package
{
    uint16_t cmd_id;
    etl::span<const uint8_t> payload;
};

/**
 * @brief 流式解析器, 带状态, 鲁棒地处理串口流
 * @tparam TOnNext 解析完成后的静态回调
 * @tparam TBufferSize 缓冲区大小
 */
class StreamParser final
{
public:
    using Observer = etl::delegate<void(raw_package)>;
    using RingBuffer = etl::circular_buffer_ext<uint8_t>;
    using TmpBuffer = etl::span<uint8_t>;

    /**
     * @brief Construct a new Stream Parser object
     * 
     * @param ring_buffer 用于临时接收数据的环形缓冲区, 需要大于单包大小
     * @param tmp_buffer 用于临时处理数据的线性缓冲区(数组), 需要大于或等于单包大小
     * @param on_next 下游通知委托
     */
    StreamParser(
        RingBuffer& ring_buffer,
        TmpBuffer tmp_buffer,
        Observer on_next
    ) : _ring_buffer(ring_buffer),
        _tmp_buffer(tmp_buffer),
        _on_next(on_next)
    {}

    /**
     * @brief 同步流式读入数据, 触发解析
     * 
     * @param data 
     */
    void feed(etl::span<const uint8_t> data);
private:
    RingBuffer& _ring_buffer;
    TmpBuffer _tmp_buffer;
    Observer _on_next;
};

}