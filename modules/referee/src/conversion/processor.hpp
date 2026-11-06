#pragma once

#include "direct.hpp"
#include "protocol/rx_types.hpp"

#include <breeze/referee/module.h>

namespace breeze::referee::internal
{

struct State final
{
	models::RobotId SelfId{};
	models::messages::LogisticsInfo LogisticsInfo{};
	models::messages::RobotHealths AllyHealths{};
	models::messages::RobotLevels AllyLevels{};
	models::messages::RobotHeats AllyHeats{};
	models::messages::RobotAmmos AllyAmmos{};
	models::messages::RobotBuffs AllyBuffs{};
	models::messages::RobotPoses AllyPoses{};
	models::messages::RobotPowers AllyPowers{};
	models::messages::GameState GameState{};
	models::messages::DartInfo DartInfo{};
};

struct Processor final
{
	Outputs& Out;
	State& Data;

	void Receive(const protocol::rx::GameStatus& value);
	void Receive(const protocol::rx::GameRobotHp& value);
	void Receive(const protocol::rx::EventData& value);
	void Receive(const protocol::rx::DartInfo& value);
	void Receive(const protocol::rx::RobotStatus& value);
	void Receive(const protocol::rx::PowerHeatData& value);
	void Receive(const protocol::rx::RobotPos& value);
	void Receive(const protocol::rx::Buff& value);
	void Receive(const protocol::rx::HurtData& value);
	void Receive(const protocol::rx::ProjectileAllowance& value);
	void Receive(const protocol::rx::RfidStatus& value);
	void Receive(const protocol::rx::GroundRobotPosition& value);
	template <typename T>
	void Receive(const T&) {}
};

} // namespace breeze::referee::internal
