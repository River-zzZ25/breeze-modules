/*
 * Copyright (c) 2026 RobotPilots-SZU
 * SPDX-License-Identifier: Apache-2.0
 */
#include "internal.h"
#include "stats.h"

#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/linker/section_tags.h>

#include <cstring>

LOG_MODULE_DECLARE(referee);

namespace breeze::referee::internal
{

struct RxMessage
{
	size_t len;
	uint8_t data[CONFIG_BREEZE_MODULE_REFEREE_SERIAL_BUFFER_SIZE];
};

struct RxContext
{
	const device *uart;
	StreamParser *parser;
	k_msgq *queue;
	uint8_t dma_buffers[2][CONFIG_BREEZE_MODULE_REFEREE_SERIAL_BUFFER_SIZE] {};
	uint8_t next_buffer;
};

K_MSGQ_DEFINE(referee_rx_queue, sizeof(RxMessage), 4, 1);
K_MSGQ_DEFINE(vt_rx_queue, sizeof(RxMessage), 4, 1);

static RxContext
	referee_context
	{ 
		.queue = &referee_rx_queue,
		.next_buffer = 1u 
	},
	vt_context
	{
		.queue = &vt_rx_queue,
		.next_buffer = 1u
	};

K_THREAD_STACK_DEFINE(referee_rx_stack, CONFIG_BREEZE_MODULE_REFEREE_RX_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(vt_rx_stack, CONFIG_BREEZE_MODULE_REFEREE_RX_THREAD_STACK_SIZE);

static k_thread referee_rx_thread, vt_rx_thread;

static void rx_handler(void *p1, void *, void *)
{
	RxContext& context = *static_cast<RxContext *>(p1);
	RxMessage message {};

	while (true)
	{
		k_msgq_get(context.queue, &message, K_FOREVER);
		context.parser->feed({message.data, message.len});
	}
}

static void uart_cb(const device *dev, uart_event *evt, void *user_data)
{
	RxContext& context = *static_cast<RxContext *>(user_data);

	switch (evt->type)
	{
		case UART_RX_BUF_REQUEST:
			uart_rx_buf_rsp(dev, context.dma_buffers[context.next_buffer],
					 sizeof(context.dma_buffers[context.next_buffer]));
			context.next_buffer ^= 1U;
			break;
		case UART_RX_RDY: {
			RxMessage message { .len = evt->data.rx.len };
			stats::record(stats::event::rx_chunks);
			stats::record(stats::event::rx_bytes, message.len);
			std::memcpy(message.data, evt->data.rx.buf + evt->data.rx.offset, message.len);
			if (k_msgq_put(context.queue, &message, K_NO_WAIT) != 0)
			{
				stats::record(stats::event::rx_queue_full);
			}
			break;
		}
		case UART_RX_STOPPED:
			stats::record(stats::event::rx_uart_stopped);
			break;
		default:
			break;
	}
}

static bool start_uart_channel(
	RxContext& context,
	const device *uart,
	StreamParser& parser,
	k_thread& thread,
	k_thread_stack_t *stack,
	size_t stack_size
)
{
	context.uart = uart;
	context.parser = &parser;
	context.next_buffer = 1U;

	if (auto ret = uart_callback_set(context.uart, uart_cb, &context); ret != 0)
	{
		LOG_ERR("referee UART callback setup failed (%d)", ret);
		return false;
	}

	if(auto ret = uart_rx_enable(context.uart, context.dma_buffers[0],
			   sizeof(context.dma_buffers[0]), 0); ret !=0)
	{
		LOG_ERR("referee UART RX enable failed (%d)", ret);
		return false;
	}

	if (k_thread_create(&thread, stack, stack_size, rx_handler, &context,
			nullptr, nullptr, CONFIG_BREEZE_MODULE_REFEREE_RX_THREAD_PRIORITY,
			0, K_NO_WAIT) == nullptr)
	{
		LOG_ERR("referee RX thread creation failed");
		return false;
	}

	return true;
}

bool start_uart(const device *uart, StreamParser& parser, bool vt)
{
	if (vt)
	{
		return start_uart_channel(
			vt_context,
			uart,
			parser,
			vt_rx_thread,
			vt_rx_stack,
		   K_THREAD_STACK_SIZEOF(vt_rx_stack)
		);
	}
	else
	{
		return start_uart_channel(
			referee_context,
			uart,
			parser,
			referee_rx_thread,
		   referee_rx_stack,
		   K_THREAD_STACK_SIZEOF(referee_rx_stack)
		);
	}
}

}
