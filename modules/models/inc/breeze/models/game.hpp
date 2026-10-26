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

#include "id.hpp"
#include "unit.hpp"
#include "utils.hpp"
#include <array>
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
 * @brief 机器人等级
 */
struct LevelType
{
	std::uint8_t Value;
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
	OverSpeed,     ///< 超弹速
	OverHeat,      ///< 超热量
	PowerOverload  ///< 超功率
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

/**
 * @brief 能量机关激活状态
 */
enum class RuneState : std::uint8_t
{
	Inactive,   ///< 未激活
	Activated,  ///< 已激活
	Activating, ///< 激活中
};

/**
 * @brief 场地占领状态
 */
enum class FieldOccupation : std::uint8_t
{
	None,  ///< 无占领
	Ally,  ///< 我方占领
	Enemy, ///< 敌方占领
	Both,  ///< 双方占领
};

/**
 * @brief 飞镖目标
 */
enum class DartTarget : std::uint8_t
{
	None,             ///< 无目标
	Outpost,          ///< 前哨站
	BaseFixed,        ///< 基地固定
	BaseRandomFixed,  ///< 基地随机固定
	BaseRandomMoving, ///< 基地随机移动
	BaseEndMoving,    ///< 基地末端移动
};

/**
 * @brief 有血量的机器人索引表
 *
 */
constexpr auto HEALTH_ROBOT_IDS = []() constexpr {
	using enum RobotShortId;
	return std::array{Hero1, Juggernaut1, Engineer2, Infantry3, Infantry4, Sentry6};
}();

/**
 * @brief 机器人的血量集合
 *
 */
struct RobotHealths : IndexedEnumArray<RobotShortId, HealthType, HEALTH_ROBOT_IDS, ROBOT_SHORT_ID_VALUES>
{};

/**
 * @brief 有发单量的机器人索引表
 *
 */
constexpr auto AMMO_ROBOT_IDS = []() constexpr {
	using enum RobotShortId;
	return std::array{Hero1, Juggernaut1, Infantry3, Infantry4, Sentry6, Drone7};
}();

/**
 * @brief 机器人的发单量集合
 *
 */
struct RobotAmmos : IndexedEnumArray<RobotShortId, AmmoType, AMMO_ROBOT_IDS, ROBOT_SHORT_ID_VALUES>
{};

constexpr auto LEVEL_ROBOT_IDS = []() constexpr {
	using enum RobotShortId;
	return std::array{Hero1, Juggernaut1, Engineer2, Infantry3, Infantry4, Sentry6, Drone7};
}();
/**
 * @brief 机器人的等级集合
 *
 */
struct RobotLevels : IndexedEnumArray<RobotShortId, LevelType, LEVEL_ROBOT_IDS, ROBOT_SHORT_ID_VALUES>
{};

/**
 * @brief 有热量的机器人索引表
 *
 */
constexpr auto HEAT_ROBOT_IDS = AMMO_ROBOT_IDS;

/**
 * @brief 机器人的热量集合
 *
 */
struct RobotHeats : IndexedEnumArray<RobotShortId, HealthType, AMMO_ROBOT_IDS, ROBOT_SHORT_ID_VALUES>
{};

} // namespace breeze::models
