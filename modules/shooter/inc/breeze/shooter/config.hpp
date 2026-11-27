/**
 * @file config.hpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief	发射模块配置文件
 * @version 0.1
 * @date 2026-11-08
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <etl/array.h>
#include <etl/span.h>
#include <etl/variant.h>

namespace breeze::shooter
{

/**
 * @brief 电机方向枚举
 *
 */
enum class MotorDirection : std::uint8_t
{
	None,    ///< 初始状态
	Forward, ///< 正转
	Reverse, ///< 反转
};

/**
 * @brief PID 系数、输入死区、积分分离与限幅配置
 *
 */
struct PidConfig
{
	float Kp; ///< 比例系数
	float Ki; ///< 积分系数, 积分累计按秒计算
	float Kd; ///< 微分系数, 误差变化率按秒计算

	float InputDeadband;           ///< 误差幅值小于该值时按零处理
	float MaxOutput;               ///< 输出幅值上限, 0 表示不限幅; 速度环为 N·m, 拨盘位置环为 °/s
	float MaxIntegral;             ///< 误差积分累计量上限(误差·s), 非 Iout 上限; 0 表示不限幅
	float InputIntegralSeparation; ///< 误差幅值超过该值时清零积分累计量; 0 表示关闭积分分离
};

/**
 * @brief 摩擦轮转速就绪配置
 *
 */
struct FricReadyConfig
{
	float SpeedRatio{};   ///< 相对容差比例, 取值 0~1, 乘以目标转速得到容差(rpm)
	float MinTolerance{}; ///< 绝对容差下限(rpm)
	float MinTarget{};    ///< 判定下限(rpm), 目标低于该值时不判 Ready
};

/**
 * @brief 摩擦轮配置结构体
 *
 */
struct FricConfig
{
	MotorDirection Direction{};    ///< 电机方向
	PidConfig Pid{};               ///< 电机pid配置
	FricReadyConfig ReadyConfig{}; ///< 转速就绪配置
};

/**
 * @brief 拨盘复位配置
 *
 */
struct FeedResetConfig
{
	float ResetSpeed{};           ///< 唤醒复位速度(°/s), 必须非零; 负值为退弹方向
	std::uint16_t ResetTimeOut{}; ///< 复位超时时间(ms)
};

/**
 * @brief 相对等距弹位, 以复位完成位置为基准
 *
 */
struct RelativeFeedPosition
{
	float Step{};            ///< 相邻弹位的拨盘轴位移(度), 大于 0 且不超过 180
	FeedResetConfig Reset{}; ///< 每次唤醒执行机械复位, 完成后建立弹位基准
};

/**
 * @brief 绝对单圈弹位表
 * @note 表值与反馈使用同一输出轴坐标, 角度在 [0, 360) 内, 不做极性转换.
 *       Positions 非空, 应用层按实际拨弹顺序传入, 下一项为下一发, 上一项为上一发.
 *       首尾循环, 相邻弹位沿 Direction 指定的拨弹方向间隔不超过 180 度.
 *       表的底层数组须在 Feed 生命周期内保持有效且不修改.
 */
struct AbsoluteFeedPosition
{
	etl::span<const float> Positions{}; ///< 一圈内按拨弹顺序排列的拨盘轴弹位(度)
};

using FeedPositionConfig = etl::variant<RelativeFeedPosition, AbsoluteFeedPosition>;

/**
 * @brief 拨弹配置
 *
 */
struct FeedShootConfig
{
	std::uint16_t ShootTimeOut{};              ///< 单次拨弹超时时间(ms)
	std::uint16_t HandleStuckReverseTimeout{}; ///< 处理堵转反转超时时间(ms)
	std::uint16_t HandleStuckReloadTimeout{};  ///< 处理堵转复位重载超时时间(ms)
	float PositionTolerance{};                 ///< 目标位置到位容差(度)
	std::uint8_t RetryMaxCount{1};             ///< 一轮清堵允许的重载堵转次数, 取值 1~255
};
/**
 * @brief 拨盘配置结构体
 *
 */
struct FeedConfig
{
	MotorDirection Direction;                            ///< 拨盘电机安装方向
	PidConfig PositionPid;                               ///< 位置环配置, 输入角度误差(°), 输出目标角速度(°/s)
	PidConfig PositionSpeedPid;                          ///< 位置环的速度内环配置, 输出力矩(N·m)
	PidConfig SpeedPid;                                  ///< 复位与连发使用的独立速度环配置, 输出力矩(N·m)
	FeedPositionConfig Position{RelativeFeedPosition{}}; ///< 相对等距弹位或绝对单圈弹位表
	FeedShootConfig Shoot;                               ///< 拨弹配置
};

/**
 * @brief 发射模块配置
 *
 * @tparam FricCount
 */
template <std::size_t FricCount>
struct ShooterConfig
{
	etl::array<FricConfig, FricCount> FricConf{}; ///< 摩擦轮配置
	float FricReverseSpeed{};                     ///< 整组清堵反转速度大小(rpm), 正值
	FeedConfig FeedConf{};                        ///< 拨盘配置
};

} // namespace breeze::shooter
