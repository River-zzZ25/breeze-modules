/**
 * @file io.hpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief 发射模块输入输出定义
 * @version 0.1
 * @date 2026-11-15
 *
 * @note 这个文件定义了发射模块的输入输出接口。
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
 * @brief 单个摩擦轮的设备反馈
 * @note 原样填写驱动返回的量, 不做方向换算:
 *       摩擦轮转速单位为 rpm, 极性与电机正转一致。
 */
struct FricFeedback
{
	float Speed{};  ///< 转速(rpm), 电机正转为正
	float Torque{}; ///< 电机输出力矩(N·m), 电机正转为正
};

/**
 * @brief 拨盘的设备反馈
 * @note 原样填写驱动返回的量, 不做方向换算:
 *       控制目标按 FeedConfig::Direction 换算到电机极性, 反馈直接参与 PID.
 *       角度为拨盘轴单圈位置 [0, 360), 保留标定零点, 不在唤醒时置零.
 *
 */
struct FeedFeedback
{
	float Speed{};  ///< 转速(rpm), 电机正转为正
	float Angle{};  ///< 拨盘轴角度 [0, 360), 电机正转方向递增, 单位为度
	float Torque{}; ///< 电机输出力矩(N·m), 电机正转为正
};

/**
 * @brief 面向应用层的整机状态
 * @note 与内部状态机 ShootState 解耦: 内部状态可以增删, 这里的取值保持稳定.
 *
 */
enum class ShooterStatus : std::uint8_t
{
	Sleep,    ///< 停机泄力
	Spinning, ///< 摩擦轮加速中
	Ready,    ///< 全部摩擦轮到速, 可发射
	Shooting, ///< 发射中
	Stuck,    ///< 卡弹
	Fault,    ///< 故障, 只能 Reset() 解除
};

/**
 * @brief 发射模块的状态报告
 *
 */
struct ShooterReport
{
	ShooterStatus Status{ShooterStatus::Sleep}; ///< 整机状态
	float TargetSpeed{};                        ///< 当前摩擦轮目标转速(rpm)
	bool FricReady{};                           ///< 所有摩擦轮已到速
};

} // namespace breeze::shooter
