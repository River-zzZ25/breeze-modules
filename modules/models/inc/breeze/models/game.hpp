/**
 * @file game.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 比赛相关信息 数据模型
 * @version 0.1
 * @date 2026-10-25
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "unit.hpp"
#include <cstdint>

namespace breeze::models
{

/**
 * @brief 比赛类型
 *
 */
enum class GameType : std::uint8_t
{
	None,
	UC, ///< 超级对抗赛
	UL, ///< 联盟塞
};

/**
 * @brief 比赛阶段
 *
 */
enum class GameStage : std::uint8_t
{
	None,
	NotStarted,  ///< 未开始
	Preparation, ///< 准备阶段 (你最喜欢的3分钟准备)
	RefereeInit, ///< 裁判系统自检
	Countdown,   ///< 比赛开始前的5秒倒计时
	InMatch,     ///< 比赛中
	Settleing    ///< 结算中 (比赛结束)
};

/**
 * @brief 判罚等级
 *
 */
enum class PenaltyLevel : std::uint8_t
{
	DualYellowCard, ///< 双方黄牌
	YellowCard,     ///< 黄牌
	RedCard,        ///< 红牌
	Forfeiture      ///< 判负
};

/**
 * @brief 血量数据类型
 *
 */
struct HealthType
{
	std::uint16_t Value;
};
/**
 * @brief 热量数据类型
 *
 */
struct HeatType
{
	uint16_t Value;
};

/**
 * @brief 发弹量类型
 *
 */
struct AmmoType
{
	uint16_t Value;
};

/**
 * @brief 平面坐标
 * @note 场地围挡在红方补给站附近的交点为坐标原点, 沿场地长边向蓝方为 X 轴正方向, 沿场地短边
向红方停机坪为 Y 轴正方向.
 *
 */
struct Position
{
	float X;
	float Y;
};

/**
 * @brief 增益/减益
 * @note 这个不是符(能量机关).
 *
 */
struct Buffs
{
	Percentage Recovery; ///< 每秒回复血量上限百分比
	HeatType Cooling;    ///< 每秒热量冷却的量
	Percentage Defence;  ///< 防御增益/减益百分比. 正为增益, 负为减益.
	Percentage Attack;   ///< 攻击增益/减益百分比. 正为增益, 负为减益.
};

/**
 * @brief 伤害源(血量变化类型)
 *
 */
enum class DamageSource : std::uint8_t
{
	Projectile,    ///< 装加班被弹丸击打
	ModuleOffline, ///< 模块离线
	Collision,     ///< 装加班被肘击
};

/**
 * @brief 单个机器人的状态集合
 * @note 本来计划是想像自定义客户端那样搞偏函数+组件的, 觉得在电控上没必要, 大结构体应该够了
 *
 */
struct RobotState
{
	HealthType Health;  ///< 当前血量
	HeatType MaxHealth; ///< 最大血量
	AmmoType Ammo;      ///< 剩余发单量
};

/**
 * @brief 经济(金币)单位
 *
 */
struct EconomyType
{
	std::uint16_t Value;
};

/**
 * @brief 后勤信息 (队伍的经济, 加密等级等等队伍级别属性)
 *
 */
struct Logistics
{
	EconomyType Economy; ///< 金币
};

} // namespace breeze::models
