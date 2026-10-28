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
enum class Event : uint8_t
{
	/** 接收到的 UART 字节数。 */
	RxBytes,
	/** UART 接收块数量。 */
	RxChunks,
	/** 写入接收消息队列失败的次数。 */
	RxQueueFull,
	/** UART 接收停止事件数量。 */
	RxUartStopped,
	/** 找到帧起始符的次数。 */
	ParserSofFound,
	/** 帧头 CRC 校验失败的次数。 */
	ParserHeaderCrcError,
	/** 帧数据长度非法的次数。 */
	ParserLengthError,
	/** 帧数据 CRC 校验失败的次数。 */
	ParserFrameCrcError,
	/** 解析成功的完整帧数量。 */
	ParserFramesOk,
	/** 解析过程中丢弃的字节数。 */
	ParserDiscardBytes,
	/** 输出裸数据包的数量。 */
	RawPackagesEmitted,
};

/**
 * @brief 初始化并注册裁判系统统计组。
 */
#ifdef CONFIG_STATS
void Init();
#else
inline void Init() {}
#endif

/**
 * @brief 增加指定调试统计项。
 *
 * @param value 统计事件。
 * @param amount 增加的数量。
 */
#ifdef CONFIG_STATS
void Record(Event value, uint32_t amount = 1U);
#else
inline void Record(Event value, uint32_t amount = 1U) {}
#endif

} // namespace breeze::referee::internal::stats
