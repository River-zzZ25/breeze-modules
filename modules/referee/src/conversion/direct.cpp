// NOLINTBEGIN(readability-magic-numbers)
#include "direct.hpp"
#include "breeze/models/id.hpp"

namespace breeze::referee::internal
{
namespace
{
using namespace models;

RobotShortId ToShortId(std::uint8_t refereeId)
{
	const auto number = static_cast<std::uint8_t>(refereeId >= 100 ? refereeId - 100 : refereeId);
	switch (number)
	{
	case 1:
		return RobotShortId::Hero1;
	case 2:
		return RobotShortId::Engineer2;
	case 3:
		return RobotShortId::Infantry3;
	case 4:
		return RobotShortId::Infantry4;
	case 5:
		return RobotShortId::Infantry5;
	case 6:
		return RobotShortId::Drone7;
	case 7:
		return RobotShortId::Sentry6;
	case 8:
		return RobotShortId::Juggernaut1;
	default:
		return RobotShortId::Unknown0;
	}
}

FieldOccupation ToFieldOccupation(std::uint8_t value)
{
	switch (value)
	{
	case 0:
		return FieldOccupation::None;
	case 1:
		return FieldOccupation::Ally;
	case 2:
		return FieldOccupation::Enemy;
	case 3:
		return FieldOccupation::Both;
	default:
		return FieldOccupation::None;
	}
}

RuneState ToRuneState(std::uint8_t value)
{
	switch (value)
	{
	case 0:
		return RuneState::Inactive;
	case 1:
		return RuneState::Activated;
	case 2:
		return RuneState::Activating;
	default:
		return RuneState::Inactive;
	}
}

DartTarget ToDartTarget(std::uint8_t value)
{
	switch (value)
	{
	case 0:
		return DartTarget::None;
	case 1:
		return DartTarget::Outpost;
	case 2:
		return DartTarget::BaseFixed;
	case 3:
		return DartTarget::BaseRandomFixed;
	case 4:
		return DartTarget::BaseRandomMoving;
	case 5:
		return DartTarget::BaseEndMoving;
	default:
		return DartTarget::None;
	}
}

DamageSource ToDamageSource(std::uint8_t value)
{
	switch (value)
	{
	case 0:
		return DamageSource::Projectile;
	case 1:
		return DamageSource::ModuleOffline;
	case 2:
		return DamageSource::Collision;
	case 3:
		return DamageSource::OverSpeed;
	case 4:
		return DamageSource::OverHeat;
	case 5:
		return DamageSource::PowerOverload;
	default:
		return DamageSource::Projectile;
	}
}

} // namespace

models::RobotId Direct::ToSelfId(const protocol::rx::RobotStatus& value)
{
	const auto team = [id = value.RobotId]() { return id > 0 ? id < 100 ? Team::Red : Team::Blue : Team::Neutral; }();
	return {ToShortId(value.RobotId), team};
}

models::messages::GameState Direct::ToGameState(const protocol::rx::GameStatus& value)
{
	using namespace models;
	messages::GameState result{};
	switch (value.GameType)
	{
	case 1:
	case 2:
	case 3:
		result.Mode = GameMode::UC;
		break;
	case 4:
	case 5:
		result.Mode = GameMode::UL;
		break;
	default:
		result.Mode = GameMode::None;
		break;
	}
	switch (value.GameProgress)
	{
	case 0:
		result.Stage = GameStage::NotStarted;
		break;
	case 1:
		result.Stage = GameStage::Preparation;
		break;
	case 2:
		result.Stage = GameStage::RefereeInit;
		break;
	case 3:
		result.Stage = GameStage::Countdown;
		break;
	case 4:
		result.Stage = GameStage::InMatch;
		break;
	case 5:
		result.Stage = GameStage::Settleing;
		break;
	default:
		result.Stage = GameStage::None;
		break;
	}
	result.Remain = {value.StageRemainTime};
	return result;
}

models::messages::DartInfo Direct::ToDartInfo(const protocol::rx::DartInfo& value)
{
	return {
	    .RemainingTime = {value.DartRemainingTime},
	    .TargetSelected = ToDartTarget((value.DartInfo >> 6) & 0x7u),
	    .HitCount = static_cast<std::uint8_t>((value.DartInfo >> 3) & 0x7u),
	};
}

models::messages::PowerManagementInfo Direct::ToPowerManagement(const protocol::rx::RobotStatus& value)
{
	return {
	    .Gimbal = value.PowerManagementGimbalOutput != 0,
	    .Chassis = value.PowerManagementChassisOutput != 0,
	    .Shooter = value.PowerManagementShooterOutput != 0,
	};
}

models::messages::OccupationEvent Direct::ToOccupation(const protocol::rx::EventData& value)
{
	using models::FieldOccupation;
	const auto bits = value.EventData;
	return {
	    .Resupply = (bits & (1u << 0)) != 0 ? FieldOccupation::Ally : FieldOccupation::None,
	    .CenterHighland = ToFieldOccupation((bits >> 7) & 0x3u),
	    .TrapezoidHighland = (bits & (1u << 9)) != 0 ? FieldOccupation::Ally : FieldOccupation::None,
	    .Fortress = ToFieldOccupation((bits >> 25) & 0x3u),
	    .Outpost = ToFieldOccupation((bits >> 27) & 0x3u),
	    .Base = (bits & (1u << 29)) != 0 ? FieldOccupation::Ally : FieldOccupation::None,
	    .Center = ToFieldOccupation((bits >> 23) & 0x3u),
	};
}

models::messages::RuneEvent Direct::ToRune(const protocol::rx::EventData& value)
{
	return {
	    .SmallState = ToRuneState((value.EventData >> 3) & 0x3u),
	    .LargeState = ToRuneState((value.EventData >> 5) & 0x3u),
	};
}

models::messages::DartHitEvent Direct::ToDartHit(const protocol::rx::EventData& value)
{
	return {
	    .LastTime = {static_cast<std::uint16_t>((value.EventData >> 11) & 0x1FFu)},
	    .LastTarget = ToDartTarget((value.EventData >> 20) & 0x7u),
	};
}

models::messages::DamageEvent Direct::ToDamage(const protocol::rx::HurtData& value)
{
	return {
	    .ArmorId = value.ArmorId,
	    .Source = ToDamageSource(value.HpDeductionReason),
	};
}

models::messages::RegionDectectionInfo Direct::ToRegion(const protocol::rx::RfidStatus& value)
{
	const auto bits = value.RfidStatus;
	return {
	    .Resupply = ((bits >> 19) & 1u) != 0 || ((bits >> 20) & 1u) != 0,
	    .CenterHighland = ((bits >> 1) & 1u) != 0,
	    .TrapezoidHighland = ((bits >> 3) & 1u) != 0,
	    .Fortress = ((bits >> 17) & 1u) != 0,
	    .Outpost = ((bits >> 18) & 1u) != 0,
	    .Base = (bits & 1u) != 0,
	    .Center = (value.RfidStatus2 & 1u) != 0,
	};
}

} // namespace breeze::referee::internal

// NOLINTEND(readability-magic-numbers)
