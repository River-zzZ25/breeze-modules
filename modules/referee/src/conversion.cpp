// NOLINTBEGIN(readability-magic-numbers)
#include "conversion.hpp"
#include "breeze/messages/game.hpp"

namespace breeze::referee::internal
{
namespace
{
using namespace models;
using namespace protocol::rx;

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

// TODO: 改用编译器展开, 引用MODEl中的常量列表, 而非手动维护

bool IsHealthSlot(RobotShortId id)
{
	switch (id)
	{
	case RobotShortId::Hero1:
	case RobotShortId::Engineer2:
	case RobotShortId::Juggernaut1:
	case RobotShortId::Infantry3:
	case RobotShortId::Infantry4:
	case RobotShortId::Sentry6:
		return true;
	default:
		return false;
	}
}

bool IsLevelSlot(RobotShortId id) { return IsHealthSlot(id) || id == RobotShortId::Drone7; }

bool IsAmmoSlot(RobotShortId id)
{
	return id == RobotShortId::Hero1 || id == RobotShortId::Juggernaut1 || id == RobotShortId::Infantry3 ||
	       id == RobotShortId::Infantry4 || id == RobotShortId::Sentry6 || id == RobotShortId::Drone7;
}

FieldOccupation ToOccupation(std::uint8_t value)
{
	return value <= static_cast<std::uint8_t>(FieldOccupation::Both) ? static_cast<FieldOccupation>(value)
	                                                                 : FieldOccupation::None;
}

RuneState ToRuneState(std::uint8_t value)
{
	return value <= static_cast<std::uint8_t>(RuneState::Activating) ? static_cast<RuneState>(value)
	                                                                 : RuneState::Inactive;
}

DartTarget ToDartTarget(std::uint8_t value)
{
	return value <= static_cast<std::uint8_t>(DartTarget::BaseEndMoving) ? static_cast<DartTarget>(value)
	                                                                     : DartTarget::None;
}

} // namespace

Conversion::Conversion()
    : In(MakeInputs(std::make_index_sequence<std::tuple_size_v<RxTypes>>{})),
      OutSelfId([this](RobotId& value) { value = _selfId; }),
      OutAllyPowers([this](messages::RobotPowers& value) { value = _allyPowers; }),
      OutGamestate([this](messages::GameState& value) { value = _gameState; }),
      OutLogisticsInfo([this](messages::LogisticsInfo& value) { value = _logisticsInfo; }),
      OutDartInfo([this](messages::DartInfo& value) { value = _dartInfo; }),
      OutAllyHealths([this](messages::RobotHealths& value) { value = _allyHealths; }),
      OutAllyLevels([this](messages::RobotLevels& value) { value = _allyLevels; }),
      OutAllyHeats([this](messages::RobotHeats& value) { value = _allyHeats; }),
      OutAllyAmmos([this](messages::RobotAmmos& value) { value = _allyAmmos; }),
      OutAllyBuffs([this](messages::RobotBuffs& value) { value = _allyBuffs; }),
      OutAllyPoses([this](messages::RobotPoses& value) { value = _allyPoses; })
{
	_dartHitPairChanged.out >> OutDartHitEvent;
}

void Conversion::Receive(const GameStatus& value)
{
	using namespace models;
	_gameState.Mode = value.GameType >= 1 && value.GameType <= 3
	                      ? GameMode::UC
	                      : (value.GameType == 4 || value.GameType == 5 ? GameMode::UL : GameMode::None);
	switch (value.GameProgress)
	{
	case 0:
		_gameState.Stage = GameStage::NotStarted;
		break;
	case 1:
		_gameState.Stage = GameStage::Preparation;
		break;
	case 2:
		_gameState.Stage = GameStage::RefereeInit;
		break;
	case 3:
		_gameState.Stage = GameStage::Countdown;
		break;
	case 4:
		_gameState.Stage = GameStage::InMatch;
		break;
	case 5:
		_gameState.Stage = GameStage::Settleing;
		break;
	default:
		_gameState.Stage = GameStage::None;
		break;
	}
	_gameState.Remain = {value.StageRemainTime};
}

void Conversion::Receive(const GameRobotHp& value)
{
	_allyHealths[RobotShortId::Hero1] = {value.Ally1RobotHp};
	_allyHealths[RobotShortId::Engineer2] = {value.Ally2RobotHp};
	_allyHealths[RobotShortId::Infantry3] = {value.Ally3RobotHp};
	_allyHealths[RobotShortId::Infantry4] = {value.Ally4RobotHp};
	_allyHealths[RobotShortId::Sentry6] = {value.Ally7RobotHp};
}

void Conversion::Receive(const EventData& value)
{
	const auto bits = value.EventData;
	const auto occupation = messages::OccupationEvent{
	    .Resupply = (bits & (1u << 0)) != 0 ? FieldOccupation::Ally : FieldOccupation::None,
	    .CenterHighland = ToOccupation((bits >> 7) & 0x3u),
	    .TrapezoidHighland = (bits & (1u << 9)) != 0 ? FieldOccupation::Ally : FieldOccupation::None,
	    .Fortress = ToOccupation((bits >> 25) & 0x3u),
	    .Outpost = ToOccupation((bits >> 27) & 0x3u),
	    .Base = (bits & (1u << 29)) != 0 ? FieldOccupation::Ally : FieldOccupation::None,
	    .Center = ToOccupation((bits >> 23) & 0x3u),
	};
	OutOccupationEvent(occupation);

	OutRuneEvent(
	    messages::RuneEvent{
	        .SmallState = ToRuneState((bits >> 3) & 0x3u),
	        .LargeState = ToRuneState((bits >> 5) & 0x3u),
	    }
	);

	_dartHitPairChanged.in(Snapshot(value));
}

void Conversion::Receive(const DartInfo& value)
{
	_dartInfo.RemainingTime = {value.DartRemainingTime};
	_dartInfo.HitCount = (value.DartInfo >> 3) & 0x7u;
	_dartInfo.TargetSelected = ToDartTarget((value.DartInfo >> 6) & 0x7u);
}

void Conversion::Receive(const RobotStatus& value)
{
	const auto shortId = ToShortId(value.RobotId);
	const auto team = value.RobotId == 0 ? Team::Neutral : (value.RobotId >= 100 ? Team::Blue : Team::Red);
	_selfId = {shortId, team};
	OutPowermanagementEvent(
	    messages::PowerManagementInfo{
	        .Gimbal = value.PowerManagementGimbalOutput != 0,
	        .Chassis = value.PowerManagementChassisOutput != 0,
	        .Shooter = value.PowerManagementShooterOutput != 0,
	    }
	);

	if (IsHealthSlot(shortId))
	{
		_allyHealths[shortId] = {value.CurrentHp};
	}
	if (IsLevelSlot(shortId))
	{
		_allyLevels[shortId] = {value.RobotLevel};
	}
	if (IsHealthSlot(shortId))
	{
		_allyPowers[shortId] = {static_cast<std::uint8_t>(value.ChassisPowerLimit)};
	}
	_logisticsInfo.CryptionLevel = {value.RobotLevel};
}

void Conversion::Receive(const PowerHeatData& value)
{
	if (IsAmmoSlot(_selfId.ShortId))
	{
		_allyHeats[_selfId.ShortId] = {value.Shooter17mmBarrelHeat};
	}
}

void Conversion::Receive(const RobotPos& value)
{
	if (_selfId.ShortId != RobotShortId::Unknown0)
	{
		_allyPoses[_selfId.ShortId] = {
		    .Pos = {value.X, value.Y},
		    .Orientation = {value.Angle},
		};
	}
}

void Conversion::Receive(const Buff& value)
{
	if (_selfId.ShortId != RobotShortId::Unknown0)
	{
		_allyBuffs[_selfId.ShortId] = {
		    .Recovery = {static_cast<std::int8_t>(value.RecoveryBuff)},
		    .Cooling = {value.CoolingBuff},
		    .Defence = {static_cast<std::int8_t>(value.DefenceBuff)},
		    .Attack = {static_cast<std::int8_t>(value.AttackBuff)},
		};
	}
}

void Conversion::Receive(const HurtData& value)
{
	if (value.HpDeductionReason <= static_cast<std::uint8_t>(DamageSource::PowerOverload))
	{
		OutDamageEvent(
		    messages::DamageEvent{
		        .ArmorId = value.ArmorId,
		        .Source = static_cast<DamageSource>(value.HpDeductionReason),
		    }
		);
	}
}

void Conversion::Receive(const ProjectileAllowance& value)
{
	_logisticsInfo.Economy = {value.RemainingGoldCoin};
	if (IsAmmoSlot(_selfId.ShortId))
	{
		_allyAmmos[_selfId.ShortId] = {
		    _selfId.ShortId == RobotShortId::Hero1 ? value.ProjectileAllowance42mm : value.ProjectileAllowance17mm
		};
	}
}

void Conversion::Receive(const RfidStatus& value)
{
	const auto bits = value.RfidStatus;
	OutRegionEvent(
	    messages::RegionDectectionInfo{
	        .Resupply = ((bits >> 19) & 1u) != 0 || ((bits >> 20) & 1u) != 0,
	        .CenterHighland = ((bits >> 1) & 1u) != 0,
	        .TrapezoidHighland = ((bits >> 3) & 1u) != 0,
	        .Fortress = ((bits >> 17) & 1u) != 0,
	        .Outpost = ((bits >> 18) & 1u) != 0,
	        .Base = (bits & 1u) != 0,
	        .Center = (value.RfidStatus2 & 1u) != 0,
	    }
	);
}

void Conversion::Receive(const GroundRobotPosition& value)
{
	_allyPoses[RobotShortId::Hero1].Pos = {value.HeroX, value.HeroY};
	_allyPoses[RobotShortId::Engineer2].Pos = {value.EngineerX, value.EngineerY};
	_allyPoses[RobotShortId::Infantry3].Pos = {value.Standard3X, value.Standard3Y};
	_allyPoses[RobotShortId::Infantry4].Pos = {value.Standard4X, value.Standard4Y};
}

messages::DartHitEvent Conversion::Snapshot(const EventData& value)
{
	return {
	    .LastTime = {static_cast<std::uint16_t>((value.EventData >> 11) & 0x1FFu)},
	    .LastTarget = ToDartTarget((value.EventData >> 20) & 0x7u),
	};
}

} // namespace breeze::referee::internal

// NOLINTEND(readability-magic-numbers)
