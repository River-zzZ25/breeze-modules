/*
 * Copyright (c) 2026 RobotPilots-SZU
 * SPDX-License-Identifier: Apache-2.0
 */
#include "internal.h"
#include "stats.h"

#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/linker/section_tags.h>
#include <zephyr/logging/log.h>

#include <cstring>

LOG_MODULE_DECLARE(referee);

namespace breeze::referee::internal
{

struct RxMessage
{
	size_t Len;
	uint8_t Data[CONFIG_BREEZE_MODULE_REFEREE_SERIAL_BUFFER_SIZE];
};

struct RxContext
{
	const device* Uart{};
	StreamParser* Parser{};
	k_msgq* Queue{};
	uint8_t DmaBuffers[2][CONFIG_BREEZE_MODULE_REFEREE_SERIAL_BUFFER_SIZE]{};
	uint8_t NextBuffer{};
};

K_MSGQ_DEFINE(refereeRxQueue, sizeof(RxMessage), 4, 1);
K_MSGQ_DEFINE(vtRxQueue, sizeof(RxMessage), 4, 1);

static RxContext refereeContext{.Queue = &refereeRxQueue, .NextBuffer = 1u},
    vtContext{.Queue = &vtRxQueue, .NextBuffer = 1u};

K_THREAD_STACK_DEFINE(refereeRxStack, CONFIG_BREEZE_MODULE_REFEREE_RX_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(vtRxStack, CONFIG_BREEZE_MODULE_REFEREE_RX_THREAD_STACK_SIZE);

static k_thread refereeRxThread, vt_rx_thread;

static void RxHandler(void* p1, void*, void*)
{
	RxContext& context = *static_cast<RxContext*>(p1);
	RxMessage message{};

	while (true)
	{
		k_msgq_get(context.Queue, &message, K_FOREVER);
		context.Parser->Feed({message.Data, message.Len});
	}
}

static void UartCb(const device* dev, uart_event* evt, void* userData)
{
	RxContext& context = *static_cast<RxContext*>(userData);

	switch (evt->type)
	{
	case UART_RX_BUF_REQUEST:
		uart_rx_buf_rsp(dev, context.DmaBuffers[context.NextBuffer], sizeof(context.DmaBuffers[context.NextBuffer]));
		context.NextBuffer ^= 1U;
		break;
	case UART_RX_RDY: {
		RxMessage message{.Len = evt->data.rx.len};
		stats::Record(stats::Event::RxChunks);
		stats::Record(stats::Event::RxBytes, message.Len);
		std::memcpy(message.Data, evt->data.rx.buf + evt->data.rx.offset, message.Len);
		if (k_msgq_put(context.Queue, &message, K_NO_WAIT) != 0)
		{
			stats::Record(stats::Event::RxQueueFull);
		}
		break;
	}
	case UART_RX_STOPPED:
		stats::Record(stats::Event::RxUartStopped);
		break;
	default:
		break;
	}
}

static bool StartUartChannel(
    RxContext& context,
    const device* uart,
    StreamParser& parser,
    k_thread& thread,
    k_thread_stack_t* stack,
    size_t stackSize
)
{
	context.Uart = uart;
	context.Parser = &parser;
	context.NextBuffer = 1U;

	if (auto ret = uart_callback_set(context.Uart, UartCb, &context); ret != 0)
	{
		LOG_ERR("referee UART callback setup failed (%d)", ret);
		return false;
	}

	if (auto ret = uart_rx_enable(context.Uart, context.DmaBuffers[0], sizeof(context.DmaBuffers[0]), 0); ret != 0)
	{
		LOG_ERR("referee UART RX enable failed (%d)", ret);
		return false;
	}

	if (k_thread_create(
	        &thread, stack, stackSize, RxHandler, &context, nullptr, nullptr,
	        CONFIG_BREEZE_MODULE_REFEREE_RX_THREAD_PRIORITY, 0, K_NO_WAIT
	    ) == nullptr)
	{
		LOG_ERR("referee RX thread creation failed");
		return false;
	}

	return true;
}

bool StartUart(const device* uart, StreamParser& parser, bool vt)
{
	if (vt)
	{
		return StartUartChannel(vtContext, uart, parser, vt_rx_thread, vtRxStack, K_THREAD_STACK_SIZEOF(vtRxStack));
	}
	else
	{
		return StartUartChannel(
		    refereeContext, uart, parser, refereeRxThread, refereeRxStack, K_THREAD_STACK_SIZEOF(refereeRxStack)
		);
	}
}

} // namespace breeze::referee::internal
