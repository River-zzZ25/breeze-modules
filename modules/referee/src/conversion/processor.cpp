#include "processor.hpp"

namespace breeze::referee::internal
{

void Processor::Receive(const protocol::rx::GameStatus& value) { Data.GameState = Direct::ToGameState(value); }
void Processor::Receive(const protocol::rx::DartInfo& value) { Data.DartInfo = Direct::ToDartInfo(value); }
void Processor::Receive(const protocol::rx::HurtData& value) { Out.DamageEvent(Direct::ToDamage(value)); }
void Processor::Receive(const protocol::rx::RfidStatus& value) { Out.RegionEvent(Direct::ToRegion(value)); }

void Processor::Receive(const protocol::rx::EventData& value)
{
	Out.OccupationEvent(Direct::ToOccupation(value));
	Out.RuneEvent(Direct::ToRune(value));
	Out.DartHitEvent.in(Direct::ToDartHit(value));
}

void Processor::Receive(const protocol::rx::GameRobotHp& value)
{
	using models::RobotShortId;
	Data.AllyHealths[RobotShortId::Hero1] = {value.Ally1RobotHp};
	Data.AllyHealths[RobotShortId::Engineer2] = {value.Ally2RobotHp};
	Data.AllyHealths[RobotShortId::Infantry3] = {value.Ally3RobotHp};
	Data.AllyHealths[RobotShortId::Infantry4] = {value.Ally4RobotHp};
	Data.AllyHealths[RobotShortId::Sentry6] = {value.Ally7RobotHp};
}

void Processor::Receive(const protocol::rx::RobotStatus& value)
{
	Data.SelfId = Direct::ToSelfId(value);
	const auto id = Data.SelfId.ShortId;
	if (auto* p = Data.AllyHealths.At(id); p)
	{
		*p = {value.CurrentHp};
	}
	if (auto* p = Data.AllyLevels.At(id); p)
	{
		*p = {value.RobotLevel};
	}
	if (auto* p = Data.AllyPowers.At(id); p)
	{
		*p = {static_cast<std::uint8_t>(value.ChassisPowerLimit)};
	}
	Data.LogisticsInfo.CryptionLevel = {value.RobotLevel};
	Out.PowerManagementEvent(Direct::ToPowerManagement(value));
}

void Processor::Receive(const protocol::rx::PowerHeatData& value)
{
	if (auto* heat = Data.AllyHeats.At(Data.SelfId.ShortId); heat)
	{
		*heat = {value.Shooter17mmBarrelHeat};
	}
}

void Processor::Receive(const protocol::rx::RobotPos& value)
{
	const auto id = Data.SelfId.ShortId;
	if (id != models::RobotShortId::Unknown0)
	{
		Data.AllyPoses[id] = {.Pos = {value.X, value.Y}, .Orientation = {value.Angle}};
	}
}

void Processor::Receive(const protocol::rx::Buff& value)
{
	const auto id = Data.SelfId.ShortId;
	if (id != models::RobotShortId::Unknown0)
	{
		Data.AllyBuffs[id] = {
		    .Recovery = {static_cast<std::int8_t>(value.RecoveryBuff)},
		    .Cooling = {value.CoolingBuff},
		    .Defence = {static_cast<std::int8_t>(value.DefenceBuff)},
		    .Attack = {static_cast<std::int8_t>(value.AttackBuff)},
		};
	}
}

void Processor::Receive(const protocol::rx::ProjectileAllowance& value)
{
	const auto id = Data.SelfId.ShortId;
	Data.LogisticsInfo.Economy = {value.RemainingGoldCoin};
	if (auto* ammo = Data.AllyAmmos.At(id); ammo)
	{
		*ammo = {id == models::RobotShortId::Hero1 ? value.ProjectileAllowance42mm : value.ProjectileAllowance17mm};
	}
}

void Processor::Receive(const protocol::rx::GroundRobotPosition& value)
{
	using models::RobotShortId;
	Data.AllyPoses[RobotShortId::Hero1].Pos = {value.HeroX, value.HeroY};
	Data.AllyPoses[RobotShortId::Engineer2].Pos = {value.EngineerX, value.EngineerY};
	Data.AllyPoses[RobotShortId::Infantry3].Pos = {value.Standard3X, value.Standard3Y};
	Data.AllyPoses[RobotShortId::Infantry4].Pos = {value.Standard4X, value.Standard4Y};
}

} // namespace breeze::referee::internal
