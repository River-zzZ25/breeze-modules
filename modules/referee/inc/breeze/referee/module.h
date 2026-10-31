/**
 * @file module.h
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 裁判系统数据流模块, 将原始字节流转换为裁判业务模型
 * @version 0.1
 * @date 2026-10-18
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <breeze/messages/game.hpp>
#include <breeze/messages/system.hpp>
#include <breeze/models/game.hpp>
#include <breeze/models/id.hpp>
#include <etl/span.h>
#include <ramen.hpp>

namespace breeze::referee
{

/**
 * @brief 裁判系统模块.
 * @par 输出接口设计原则
 * - 低频数据使用触发式 (Pusher), 在调用输入端口的上下文中处理
 * - 高频数据使用拉取式 (Pullable)
 */
struct Module
{
	ramen::Pullable<models::RobotId> OutSelfId;
	ramen::Pusher<models::messages::PowerManagementInfo> OutPowermanagementEvent;
	ramen::Pusher<models::messages::RegionDectectionInfo> OutRegionEvent;
	ramen::Pullable<models::messages::RobotPowers> OutAllyPowers;

	ramen::Pullable<models::messages::GameState> OutGamestate;
	ramen::Pullable<models::messages::LogisticsInfo> OutLogisticsInfo;

	ramen::Pusher<models::messages::DamageEvent> OutDamageEvent;
	ramen::Pusher<models::messages::OccupationEvent> OutOccupationEvent;
	ramen::Pusher<models::messages::RuneEvent> OutRuneEvent;
	ramen::Pusher<models::messages::DartHitEvent> OutDartHitEvent;
	ramen::Pullable<models::messages::DartInfo> OutDartInfo;

	ramen::Pullable<models::messages::RobotHealths> OutAllyHealths;
	ramen::Pullable<models::messages::RobotLevels> OutAllyLevels;
	ramen::Pullable<models::messages::RobotHeats> OutAllyHeats;
	ramen::Pullable<models::messages::RobotAmmos> OutAllyAmmos;
	ramen::Pullable<models::messages::RobotBuffs> OutAllyBuffs;
	ramen::Pullable<models::messages::RobotPoses> OutAllyPoses;

	/// @brief 裁判系统原始数据输入
	ramen::Pushable<etl::span<const uint8_t>> InReferee;
	/// @brief 图传原始数据输入
	ramen::Pushable<etl::span<const uint8_t>> InVT;

	Module();
	Module(const Module&) = delete;
	Module(Module&&) = delete;
	Module& operator=(const Module&) = delete;
	Module& operator=(Module&&) = delete;
};

extern Module module;

} // namespace breeze::referee
