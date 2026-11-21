/**
 * @file state.hpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief
 * @version 0.1
 * @date 2026-11-14
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <cstdint>

namespace breeze::shooter
{

/**
 * @brief 摩擦轮工作模式枚举
 *
 */
enum class FricState : std::uint8_t
{
	Sleep,   ///< 休眠(泄力)
	Stop,    ///< 停止(控速0)
	Running, ///< 正常运行
	Stuck,   ///< 堵转
	Reverse, ///< 反转
};

/**
 * @brief 拨盘堵转状态枚举
 *
 */
enum class FeedState : std::uint8_t
{
	Sleep,   ///< 休眠(泄力)
	Running, ///< 正常
	Stuck,   ///< 堵转
	Reverse, ///< 反转
	Reload,  ///< 复位重载
	Init,    ///< 唤醒后速度环复位
	SingleShoot, ///< 位置环单发
	RepeatShoot  ///< 速度环连发
};

/**
 * @brief 发射模块状态枚举
 *
 */
enum class ShootState : std::uint8_t
{
	Sleep,           ///< 睡眠状态：摩擦轮拨盘卸力
	Init,            ///< 初始化状态：摩擦轮开转，拨盘速度环反转复位
	Waiting,         ///< 等待就绪状态：拨盘角度环，待命中
	SingleShoot,     ///< 单发发射状态：拨盘角度换，执行单次拨弹
	RepeatShoot,     ///< 连发发射状态：拨盘速度环，执行连续拨弹
	HandleFeedStuck, ///< 卡弹处理状态：拨盘角度环反转退弹
	HandleReloading, ///< 复位重载状态：拨盘角度环复位重载上弹
	HandleFricStuck, ///< 摩擦轮堵转处理状态：摩擦轮反转防堵,处理摩擦轮的异常堵转;
	Fault            ///< 故障状态：摩擦轮或拨盘异常堵转，无法恢复
};

} // namespace breeze::shooter
