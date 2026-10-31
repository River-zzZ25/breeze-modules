#pragma once

#include "protocol.hpp"
#include "referee.h"

#include <tuple>

namespace breeze::referee::protocol::rx
{

using Types = std::tuple<
    GameStatus,
    GameRobotHp,
    EventData,
    DartInfo,
    RobotStatus,
    PowerHeatData,
    RobotPos,
    Buff,
    HurtData,
    ShootData,
    ProjectileAllowance,
    RfidStatus,
    GroundRobotPosition,
    SentryInfo,
    CustomControllerData,
    MapCommand,
    MapData,
    CustomInfo,
    CustomClientData
>;

template <typename T>
struct ReceiverFor;

template <typename... Protocols>
struct ReceiverFor<std::tuple<Protocols...>>
{
	using Type = Protocol<Protocols...>;
};

using Receiver = typename ReceiverFor<Types>::Type;

} // namespace breeze::referee::protocol::rx
