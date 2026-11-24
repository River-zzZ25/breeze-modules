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

#include <cstdint>
#include <etl/array.h>
#include <etl/circular_buffer.h>
#include <etl/delegate.h>
#include <etl/optional.h>
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
 * @brief pid配置
 * @note 这是一个没有完善的结构体
 *
 */
struct PidConfig
{
	float Kp; // 比例系数
	float Ki; // 积分系数
	float Kd; // 微分系数

	float InputDeadband;           // 输入死区
	float MaxOutput;               // 最大输出幅值, 0 表示不限幅; Fric 为 N·m
	float MaxIntegral;             // 误差积分累计量上限(误差·s), 非 Iout 上限; 0 表示不限幅
	float InputIntegralSeparation; // 积分分离值
};

/**
 * @brief 摩擦轮堵转配置
 *
 */
struct FricStuckConfig
{
	float TorqueThreshold{};       ///< 出弹方向力矩幅值阈值(N·m)
	float SpeedThreshold{};        ///< 转速幅值阈值(rpm)
	std::uint16_t ConfirmCycles{}; ///< 连续满足条件的控制周期数; 必须不小于 1
};
/**
 * @brief 摩擦轮转速就绪配置
 *
 */
struct FricReadyConfig
{
	float SpeedRatio{};   ///< 相对容差, 目标转速的百分比，输入0~1之间的数值，表示目标转速的百分比
	float MinTolerance{}; ///< 绝对容差下限(rpm)
	float MinTarget{};    ///< 判定下限(rpm), 目标低于该值时不判 Ready
};

/**
 * @brief 摩擦轮配置结构体
 *
 */
struct FricConfig
{
	MotorDirection Direction{};                   ///< 电机方向
	PidConfig Pid{};                              ///< 电机pid配置
	etl::optional<FricStuckConfig> StuckConfig{}; ///< 堵转配置
	FricReadyConfig ReadyConfig{};                ///< 转速就绪配置
};

/**
 * @brief  堵转检测配置
 *
 */
struct FeedStuckConfig
{
	float TorqueThreshold{};   ///< 堵转判定力矩阈值(N·m)
	float SpeedThreshold{};    ///< 堵转判定速度阈值(rpm)
	std::uint16_t ConfirmMs{}; ///< 堵转确认持续时间(ms)
};

/**
 * @brief 拨盘复位配置
 *
 */
struct FeedResetConfig
{
	float ResetSpeed{};           ///< 唤醒复位速度(rpm), 必须非零; 负值为退弹方向
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
};
/**
 * @brief 拨盘配置结构体
 *
 */
struct FeedConfig
{
	MotorDirection Direction;                            ///< 拨盘电机安装方向
	PidConfig PositionPid;                               ///< 位置环配置, 输出目标转速(rpm)
	PidConfig PositionSpeedPid;                          ///< 速度环配置, 输出力矩(N·m)
	PidConfig SpeedPid;                                  ///< 速度环配置, 输出力矩(N·m)
	FeedStuckConfig Stuck;                               ///< 堵转配置
	FeedPositionConfig Position{RelativeFeedPosition{}}; ///< 相对等距弹位或绝对单圈弹位表
	FeedShootConfig Shoot;                               ///< 拨弹配置
};

/**
 * @brief 弹速自适应配置
 *
 * @note 该配置用于根据理想弹速和摩擦轮转速来调整射击参数。
 *		在没有设置其他属性时，默认使用初始摩擦轮目标转速进行射击。
 */
struct ShootSpeedAdaptConfig
{
	bool Enable{};                   ///< 是否启用弹速自适应
	float InintialFricTargetSpeed{}; ///< 初始摩擦轮目标转速(rpm)
	float LobInitialFricSpeed{};     ///< 吊射初始摩擦轮转速(rpm)
	float IdealSpeedMin{};           ///< 理想弹速最小值(m/s)
	float IdealSpeedMax{};           ///< 理想弹速最大值(m/s)
	float FricSpeedMin{};            ///< 摩擦轮最小转速(rpm)
	float FricSpeedMax{};            ///< 摩擦轮最大转速(rpm)
	float OverSpeedAdjustment{};     ///< 超速时摩擦轮转速调整量
	float LowSpeedAdjustment{};      ///< 欠速时摩擦轮转速调整量
	uint16_t LowSpeedMaxCount{};     ///< 欠速最大计数，超过这个计数就认为是欠速
	float HighSpeedAdjustment{};     ///< 高速时摩擦轮转速调整量
	uint16_t HighSpeedMaxCount{};    ///< 高速最大计数，超过这个计数就认为是高速
};

/**
 * @brief 热量限制配置
 *
 * @note 两项检测由对应标志启用; 未填写的数值默认为零.
 */
struct ShootHeatLimitConfig
{
	bool AllowShootCountEnable{};     ///< 是否启用剩余发弹量检测
	bool HeatLimitEnable{};           ///< 是否启用热量限制
	float FeedSpeedCoefficient{};     ///< 连发射频到拨盘转速的换算系数(rpm/Hz)
	float HighHeatShootingFreq{};     ///< 剩余热量充足或未启用热量限制时的连发射频(Hz)
	std::uint16_t LowHeatValue{};     ///< 剩余热量低档阈值
	float LowHeatShootingFreq{};      ///< 剩余热量低档连发射频(Hz)
	std::uint16_t VeryLowHeatValue{}; ///< 剩余热量极低档阈值
	float VeryLowHeatShootingFreq{};  ///< 剩余热量极低档连发射频(Hz)
	std::uint16_t NoShootHeatValue{}; ///< 剩余热量停止发射阈值
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
	ShootSpeedAdaptConfig SpeedAdaptConf{};       ///< 弹速自适应配置
	ShootHeatLimitConfig HeatLimitConf{};         ///< 热量限制配置
};

} // namespace breeze::shooter
