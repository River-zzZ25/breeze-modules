#pragma once

#include "protocol/referee.h"

#include <breeze/messages/game.hpp>
#include <breeze/messages/system.hpp>

namespace breeze::referee::internal
{

/// 单个协议包到领域模型的无状态映射, 不持有融合状态或公开端口副本.
struct Direct final
{
	static models::RobotId ToSelfId(const protocol::rx::RobotStatus& value);
	static models::messages::GameState ToGameState(const protocol::rx::GameStatus& value);
	static models::messages::DartInfo ToDartInfo(const protocol::rx::DartInfo& value);
	static models::messages::PowerManagementInfo ToPowerManagement(const protocol::rx::RobotStatus& value);
	static models::messages::OccupationEvent ToOccupation(const protocol::rx::EventData& value);
	static models::messages::RuneEvent ToRune(const protocol::rx::EventData& value);
	static models::messages::DartHitEvent ToDartHit(const protocol::rx::EventData& value);
	static models::messages::DamageEvent ToDamage(const protocol::rx::HurtData& value);
	static models::messages::RegionDectectionInfo ToRegion(const protocol::rx::RfidStatus& value);
};

} // namespace breeze::referee::internal
