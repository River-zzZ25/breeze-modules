/**
 * @file state.hpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief 摩擦轮、拨盘与发射控制器的内部工作状态
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
	Reverse, ///< 反转
};

/**
 * @brief 拨盘工作状态枚举
 *
 */
enum class FeedState : std::uint8_t
{
	Sleep,       ///< 休眠(泄力)
	Running,     ///< 位置环待命
	Stuck,       ///< 堵转
	Reverse,     ///< 反转
	Reload,      ///< 退弹后正向重载
	Init,        ///< 唤醒后速度环复位
	SingleShoot, ///< 位置环单发
	RepeatShoot  ///< 速度环连发
};

/**
 * @brief 发射模块状态枚举
 *
 */
enum class ShootState : std::uint8_t
{
	Sleep,  ///< 摩擦轮与拨盘卸力
	Init,   ///< 摩擦轮加速, 拨盘复位或对齐
	Running ///< 准备完成, 按应用层命令控制拨弹
};

} // namespace breeze::shooter
