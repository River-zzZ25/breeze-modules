/*
 * Copyright (c) 2026 RobotPilots-SZU
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <cstdint>

/**
 * @brief 裁判系统内部调试统计。
 */
namespace breeze::referee::internal::stats
{

/**
 * @brief 裁判系统统计事件
 */
enum class event : uint8_t
{
	/** 接收到的 UART 字节数。 */
	rx_bytes,
	/** UART 接收块数量。 */
	rx_chunks,
	/** 写入接收消息队列失败的次数。 */
	rx_queue_full,
	/** UART 接收停止事件数量。 */
	rx_uart_stopped,
	/** 找到帧起始符的次数。 */
	parser_sof_found,
	/** 帧头 CRC 校验失败的次数。 */
	parser_header_crc_error,
	/** 帧数据长度非法的次数。 */
	parser_length_error,
	/** 帧数据 CRC 校验失败的次数。 */
	parser_frame_crc_error,
	/** 解析成功的完整帧数量。 */
	parser_frames_ok,
	/** 解析过程中丢弃的字节数。 */
	parser_discard_bytes,
	/** 输出裸数据包的数量。 */
	raw_packages_emitted,
};

/**
 * @brief 初始化并注册裁判系统统计组。
 */
#ifdef CONFIG_STATS
void init();
#else
inline void init() {}
#endif

/**
 * @brief 增加指定调试统计项。
 *
 * @param value 统计事件。
 * @param amount 增加的数量。
 */
#ifdef CONFIG_STATS
void record(event value, uint32_t amount = 1U);
#else
inline void record(event value, uint32_t amount = 1U) {}
#endif

}
