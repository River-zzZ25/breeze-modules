#pragma once

#include "protocol/rx_types.hpp"

#include <breeze/messages/game.hpp>
#include <breeze/messages/system.hpp>
#include <ramenx/logic.hpp>
#include <tuple>
#include <utility>

namespace breeze::referee::internal
{

class Conversion final
{
	using RxTypes = protocol::rx::Types;
	template <std::size_t... Is>
	static auto MakeInputTypes(std::index_sequence<Is...>)
	    -> std::tuple<ramen::Pushable<std::tuple_element_t<Is, RxTypes>>...>;

private:
	template <typename T>
	void Receive(const T&)
	{}

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

public:
	using Inputs = decltype(MakeInputTypes(std::make_index_sequence<std::tuple_size_v<RxTypes>>{}));

	Inputs In;

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

	Conversion();
	Conversion(const Conversion&) = delete;
	Conversion(Conversion&&) = delete;
	Conversion& operator=(const Conversion&) = delete;
	Conversion& operator=(Conversion&&) = delete;

private:
	models::RobotId _selfId{models::RobotShortId::Unknown0, models::Team::Neutral};
	models::messages::GameState _gameState{};
	models::messages::LogisticsInfo _logisticsInfo{};
	models::messages::DartInfo _dartInfo{};
	models::messages::RobotHealths _allyHealths{};
	models::messages::RobotLevels _allyLevels{};
	models::messages::RobotHeats _allyHeats{};
	models::messages::RobotAmmos _allyAmmos{};
	models::messages::RobotBuffs _allyBuffs{};
	models::messages::RobotPoses _allyPoses{};
	models::messages::RobotPowers _allyPowers{};

	ramen::Property<models::messages::DartHitEvent> _dartHitPairChanged{{}};

	template <typename T>
	ramen::Pushable<T> MakeInput()
	{
		return [this](const T& value) { Receive(value); };
	}

	template <std::size_t... Is>
	Inputs MakeInputs(std::index_sequence<Is...>)
	{
		return {MakeInput<std::tuple_element_t<Is, RxTypes>>()...};
	}

	static models::messages::DartHitEvent Snapshot(const protocol::rx::EventData& value);
};

} // namespace breeze::referee::internal
