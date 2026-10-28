#include <breeze/msgpipe/sub.hpp>
#include <zephyr/zbus/zbus.h>

#include "breeze/msgpipe/pub.hpp"
#include "breeze/referee/messages.h"
#include "breeze/referee/msgpipe.h"
#include "protocol/parser.h"
#include "protocol/referee.h"

namespace breeze::referee
{
namespace internal
{

// 实例化裸包结构体和zbus内结构体
static protocol::rx::JudgeInfo judgeInfo;
static struct GameStateInfo gameState;
static struct RobotStateInfo robotState;
static struct ChassisStateInfo chassisState;
static struct ShooterStateInfo shooterState;
static struct PlayerClientRxInfo playerClient;
static struct SentryOnlyInfo sentry;
static struct CustomControllerRxInfo customController;
static struct CustomClientRxInfo customClient;

void OnRawPackage(RawPackage pkg)
{
	switch (pkg.CmdId)
	{
	// 0x0001 比赛状态
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::GameStatus): {
		memcpy(
		    &judgeInfo.GameStatus, pkg.Payload.data(), static_cast<size_t>(protocol::rx::RefereeRxLength::GameStatus)
		);

		gameState.GameType = judgeInfo.GameStatus.GameType;
		gameState.GameProgress = judgeInfo.GameStatus.GameProgress;
		gameState.StageRemainTime = judgeInfo.GameStatus.StageRemainTime;

		msgpipe::Publish(gameState, K_NO_WAIT);
		break;
	}

	// 0x0003 机器人血量
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::GameRobotHp): {
		memcpy(
		    &judgeInfo.GameRobotHp, pkg.Payload.data(), static_cast<size_t>(protocol::rx::RefereeRxLength::GameRobotHp)
		);

		gameState.AllyHeroHp = judgeInfo.GameRobotHp.Ally1RobotHp;
		gameState.AllyEngineerHp = judgeInfo.GameRobotHp.Ally2RobotHp;
		gameState.AllyInfantry3Hp = judgeInfo.GameRobotHp.Ally3RobotHp;
		gameState.AllyInfantry4Hp = judgeInfo.GameRobotHp.Ally4RobotHp;
		gameState.AllySentryHp = judgeInfo.GameRobotHp.Ally7RobotHp;
		gameState.AllyOutpostHp = judgeInfo.GameRobotHp.AllyOutpostHp;
		gameState.AllyBaseHp = judgeInfo.GameRobotHp.AllyBaseHp;
		gameState.EnemyOutpostHp = judgeInfo.GameRobotHp.EnemyOutpostHp;
		gameState.EnemyBaseHp = judgeInfo.GameRobotHp.EnemyBaseHp;

		msgpipe::Publish(gameState, K_NO_WAIT);
		break;
	}

	// 0x0101 场地事件
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::EventData): {
		memcpy(&judgeInfo.EventData, pkg.Payload.data(), static_cast<size_t>(protocol::rx::RefereeRxLength::EventData));

		const uint32_t bits = judgeInfo.EventData.EventData;
		gameState.AllySupplyOcc = (bits >> 0) & 1u;
		gameState.AllySupplyOccRmul = (bits >> 2) & 1u;
		gameState.AllySmallEnergyMechanismState = (bits >> 3) & 3u;
		gameState.AllyBigEnergyMechanismState = (bits >> 5) & 3u;
		gameState.AllyCenterHighlandOcc = (bits >> 7) & 3u;
		gameState.AllyTrapezoidHighlandOcc = (bits >> 9) & 1u;
		gameState.EnemyDartLastHit = (bits >> 20) & 7u;
		gameState.AllyCenterBuffOccRmul = (bits >> 23) & 3u;
		gameState.AllyFortBuffOcc = (bits >> 25) & 3u;
		gameState.AllyOutpostBuffOcc = (bits >> 27) & 3u;
		gameState.AllyBaseBuffOcc = (bits >> 29) & 1u;

		msgpipe::Publish(gameState, K_NO_WAIT);
		break;
	}

	// 0x0105 飞镖发射
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::DartInfo): {
		memcpy(&judgeInfo.DartInfo, pkg.Payload.data(), static_cast<size_t>(protocol::rx::RefereeRxLength::DartInfo));

		gameState.AllyDartRemainingTime = judgeInfo.DartInfo.DartRemainingTime;
		gameState.AllyDartLastHitTarget = (judgeInfo.DartInfo.DartInfo >> 0) & 7u;
		gameState.LastTargetHitTimes = (judgeInfo.DartInfo.DartInfo >> 3) & 7u;
		gameState.LastTargetHitType = (judgeInfo.DartInfo.DartInfo >> 6) & 7u;

		msgpipe::Publish(gameState, K_NO_WAIT);
		break;
	}

	// 0x0201 机器人性能体系数据
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::RobotStatus): {
		memcpy(
		    &judgeInfo.RobotStatus, pkg.Payload.data(), static_cast<size_t>(protocol::rx::RefereeRxLength::RobotStatus)
		);

		robotState.RobotId = judgeInfo.RobotStatus.RobotId;
		robotState.RobotLevel = judgeInfo.RobotStatus.RobotLevel;
		robotState.CurrentHp = judgeInfo.RobotStatus.CurrentHp;
		robotState.MaximumHp = judgeInfo.RobotStatus.MaximumHp;

		chassisState.ChassisPowerLimit = judgeInfo.RobotStatus.ChassisPowerLimit;

		shooterState.ShooterBarrelCoolingValue = judgeInfo.RobotStatus.ShooterBarrelCoolingValue;
		shooterState.ShooterBarrelHeatLimit = judgeInfo.RobotStatus.ShooterBarrelHeatLimit;
		shooterState.BulletSpeedLimit = judgeInfo.RobotStatus.BulletSpeedLimit;

		msgpipe::Publish(robotState, K_NO_WAIT);
		msgpipe::Publish(chassisState, K_NO_WAIT);
		msgpipe::Publish(shooterState, K_NO_WAIT);
		break;
	}

	// 0x0202 实时底盘缓冲能量和射击热量数据
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::PowerHeatData): {
		memcpy(
		    &judgeInfo.PowerHeatData, pkg.Payload.data(),
		    static_cast<size_t>(protocol::rx::RefereeRxLength::PowerHeatData)
		);

		chassisState.BufferEnergy = judgeInfo.PowerHeatData.BufferEnergy;

		shooterState.Shooter17mmBarrelHeat = judgeInfo.PowerHeatData.Shooter17mmBarrelHeat;
		shooterState.Shooter42mmBarrelHeat = judgeInfo.PowerHeatData.Shooter42mmBarrelHeat;

		msgpipe::Publish(chassisState, K_NO_WAIT);
		msgpipe::Publish(shooterState, K_NO_WAIT);
		break;
	}

	// 0x0203 机器人位置数据
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::RobotPos): {
		memcpy(&judgeInfo.RobotPos, pkg.Payload.data(), static_cast<size_t>(protocol::rx::RefereeRxLength::RobotPos));

		robotState.X = judgeInfo.RobotPos.X;
		robotState.Y = judgeInfo.RobotPos.Y;
		robotState.Angle = judgeInfo.RobotPos.Angle;

		msgpipe::Publish(robotState, K_NO_WAIT);
		break;
	}

	// 0x0204 机器人增益和底盘能量数据
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::Buff): {
		memcpy(&judgeInfo.Buff, pkg.Payload.data(), static_cast<size_t>(protocol::rx::RefereeRxLength::Buff));

		robotState.RecoveryBuff = judgeInfo.Buff.RecoveryBuff;
		robotState.RemainingEnergy = judgeInfo.Buff.RemainingEnergy;

		msgpipe::Publish(robotState, K_NO_WAIT);
		break;
	}

	// 0x0206 伤害状态数据
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::HurtData): {
		memcpy(&judgeInfo.HurtData, pkg.Payload.data(), static_cast<size_t>(protocol::rx::RefereeRxLength::HurtData));

		robotState.ArmorId = judgeInfo.HurtData.ArmorId;
		robotState.HpDeductionReason = judgeInfo.HurtData.HpDeductionReason;

		msgpipe::Publish(robotState, K_NO_WAIT);
		break;
	}

	// 0x0207 实时射击数据
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::ShootData): {
		memcpy(&judgeInfo.ShootData, pkg.Payload.data(), static_cast<size_t>(protocol::rx::RefereeRxLength::ShootData));

		shooterState.LaunchingFrequency = judgeInfo.ShootData.LaunchingFrequency;
		shooterState.InitialSpeed = judgeInfo.ShootData.InitialSpeed;

		msgpipe::Publish(shooterState, K_NO_WAIT);
		break;
	}

	// 0x0208 允许发弹量与剩余金币
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::ProjectileAllowance): {
		memcpy(
		    &judgeInfo.ProjectileAllowance, pkg.Payload.data(),
		    static_cast<size_t>(protocol::rx::RefereeRxLength::ProjectileAllowance)
		);

		robotState.RemainingGoldCoin = judgeInfo.ProjectileAllowance.RemainingGoldCoin;

		shooterState.ProjectileAllowance17mm = judgeInfo.ProjectileAllowance.ProjectileAllowance17mm;
		shooterState.ProjectileAllowance42mm = judgeInfo.ProjectileAllowance.ProjectileAllowance42mm;

		msgpipe::Publish(robotState, K_NO_WAIT);
		msgpipe::Publish(shooterState, K_NO_WAIT);
		break;
	}

	// 0x0209 机器人 RFID 模块状态
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::RfidStatus): {
		memcpy(
		    &judgeInfo.RfidStatus, pkg.Payload.data(), static_cast<size_t>(protocol::rx::RefereeRxLength::RfidStatus)
		);

		const uint32_t rfid = judgeInfo.RfidStatus.RfidStatus;
		const uint8_t rfid2 = judgeInfo.RfidStatus.RfidStatus2;
		robotState.AllyBaseBuff = (rfid >> 0) & 1u;
		robotState.AllyCentralHighlandBuff = (rfid >> 1) & 1u;
		robotState.EnemyCentralHighlandBuff = (rfid >> 2) & 1u;
		robotState.AllyTrapezoidHighlandBuff = (rfid >> 3) & 1u;
		robotState.EnemyTrapezoidHighlandBuff = (rfid >> 4) & 1u;
		robotState.AllyFlySlopeBefore = (rfid >> 5) & 1u;
		robotState.AllyFlySlopeAfter = (rfid >> 6) & 1u;
		robotState.EnemyFlySlopeBefore = (rfid >> 7) & 1u;
		robotState.EnemyFlySlopeAfter = (rfid >> 8) & 1u;
		robotState.AllyCentralHighlandBelow = (rfid >> 9) & 1u;
		robotState.AllyCentralHighlandAbove = (rfid >> 10) & 1u;
		robotState.EnemyCentralHighlandBelow = (rfid >> 11) & 1u;
		robotState.EnemyCentralHighlandAbove = (rfid >> 12) & 1u;
		robotState.AllyHighwayBelow = (rfid >> 13) & 1u;
		robotState.AllyHighwayAbove = (rfid >> 14) & 1u;
		robotState.EnemyHighwayBelow = (rfid >> 15) & 1u;
		robotState.EnemyHighwayAbove = (rfid >> 16) & 1u;
		robotState.AllyFortBuff = (rfid >> 17) & 1u;
		robotState.AllyOutpostBuff = (rfid >> 18) & 1u;
		robotState.AllySupplyNoOverlap = (rfid >> 19) & 1u;
		robotState.AllySupplyOverlap = (rfid >> 20) & 1u;
		robotState.AllyAssemblyBuff = (rfid >> 21) & 1u;
		robotState.EnemyAssemblyBuff = (rfid >> 22) & 1u;
		robotState.CenterBuffRmul = (rfid >> 23) & 1u;
		robotState.EnemyFortBuff = (rfid >> 24) & 1u;
		robotState.EnemyOutpostBuff = (rfid >> 25) & 1u;
		robotState.AllyTunnelHighwayBelow = (rfid >> 26) & 1u;
		robotState.AllyTunnelHighwayMid = (rfid >> 27) & 1u;
		robotState.AllyTunnelHighwayAbove = (rfid >> 28) & 1u;
		robotState.AllyTunnelHighlandBelow = (rfid >> 29) & 1u;
		robotState.AllyTunnelHighlandMid = (rfid >> 30) & 1u;
		robotState.AllyTunnelHighlandAbove = (rfid >> 31) & 1u;
		robotState.EnemyTunnelHighwayBelow = (rfid2 >> 0) & 1u;
		robotState.EnemyTunnelHighwayMid = (rfid2 >> 1) & 1u;
		robotState.EnemyTunnelHighwayAbove = (rfid2 >> 2) & 1u;
		robotState.EnemyTunnelHighlandBelow = (rfid2 >> 3) & 1u;
		robotState.EnemyTunnelHighlandMid = (rfid2 >> 4) & 1u;
		robotState.EnemyTunnelHighlandAbove = (rfid2 >> 5) & 1u;

		msgpipe::Publish(robotState, K_NO_WAIT);
		break;
	}

	// 0x020B 地面机器人位置数据
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::GroundRobotPos): {
		memcpy(
		    &judgeInfo.GroundRobotPos, pkg.Payload.data(),
		    static_cast<size_t>(protocol::rx::RefereeRxLength::GroundRobotPos)
		);

		sentry.AllyHeroX = judgeInfo.GroundRobotPos.HeroX;
		sentry.AllyHeroY = judgeInfo.GroundRobotPos.HeroY;
		sentry.AllyEngineerX = judgeInfo.GroundRobotPos.EngineerX;
		sentry.AllyEngineerY = judgeInfo.GroundRobotPos.EngineerY;
		sentry.AllyInfantry3X = judgeInfo.GroundRobotPos.Standard3X;
		sentry.AllyInfantry3Y = judgeInfo.GroundRobotPos.Standard3Y;
		sentry.AllyInfantry4X = judgeInfo.GroundRobotPos.Standard4X;
		sentry.AllyInfantry4Y = judgeInfo.GroundRobotPos.Standard4Y;

		msgpipe::Publish(sentry, K_NO_WAIT);
		break;
	}

	// 0x020D 哨兵自主决策信息同步
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::SentryInfo): {
		memcpy(
		    &judgeInfo.SentryInfo, pkg.Payload.data(), static_cast<size_t>(protocol::rx::RefereeRxLength::SentryInfo)
		);

		const uint32_t info = judgeInfo.SentryInfo.SentryInfo;
		const uint16_t info2 = judgeInfo.SentryInfo.SentryInfo2;
		const uint64_t info3 = judgeInfo.SentryInfo.SentryInfo3;
		sentry.ExchangeProjectileAllowance = (info >> 0) & 0x7FFu;
		sentry.RemoteExchangeProjectileAllowanceTimes = (info >> 11) & 0xFu;
		sentry.RemoteExchangeHpTimes = (info >> 15) & 0xFu;
		sentry.FreeReviveAvailable = (info >> 19) & 1u;
		sentry.InstantReviveAvailable = (info >> 20) & 1u;
		sentry.InstantReviveCost = (info >> 21) & 0x3FFu;
		sentry.OutOfCombat = (info2 >> 0) & 1u;
		sentry.TeamProjectileAllowance17mmLeft = (info2 >> 1) & 0x7FFu;
		sentry.Posture = (info2 >> 12) & 3u;
		sentry.EnergyMechanismActivateAllowance = (info2 >> 14) & 1u;
		sentry.EnhancedPosture = (info2 >> 15) & 1u;
		sentry.AttackBeforeWeakTimeLeft = (info3 >> 0) & 0xFFu;
		sentry.DefenseBeforeWeakTimeLeft = (info3 >> 8) & 0xFFu;
		sentry.MoveBeforeWeakTimeLeft = (info3 >> 16) & 0xFFu;
		sentry.EnhancedAttackTimeLeft = (info3 >> 32) & 0xFFu;
		sentry.EnhancedDefenseTimeLeft = (info3 >> 40) & 0xFFu;
		sentry.EnhancedMoveTimeLeft = (info3 >> 48) & 0xFFu;

		msgpipe::Publish(sentry, K_NO_WAIT);
		break;
	}

	// 0x0302 自定义控制器 -> 机器人（图传）
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::CustomControllerData): {
		memcpy(
		    &judgeInfo.CustomControllerData, pkg.Payload.data(),
		    static_cast<size_t>(protocol::rx::RefereeRxLength::CustomControllerData)
		);

		memcpy(
		    customController.CustomControllerData, judgeInfo.CustomControllerData.Data,
		    sizeof(customController.CustomControllerData)
		);

		msgpipe::Publish(customController, K_NO_WAIT);
		break;
	}

	// 0x0303 选手端 -> 机器人（图传）
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::MapCommand): {
		memcpy(
		    &judgeInfo.MapCommand, pkg.Payload.data(), static_cast<size_t>(protocol::rx::RefereeRxLength::MapCommand)
		);

		playerClient.TargetPositionX = judgeInfo.MapCommand.TargetPositionX;
		playerClient.TargetPositionY = judgeInfo.MapCommand.TargetPositionY;
		playerClient.CmdKeyboard = judgeInfo.MapCommand.CmdKeyboard;
		playerClient.TargetRobotId = judgeInfo.MapCommand.TargetRobotId;
		playerClient.CmdSource = judgeInfo.MapCommand.CmdSource;

		msgpipe::Publish(playerClient, K_NO_WAIT);
		break;
	}

	// 0x0307 选手端小地图接收路径数据
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::MapData): {
		memcpy(&judgeInfo.MapData, pkg.Payload.data(), static_cast<size_t>(protocol::rx::RefereeRxLength::MapData));

		sentry.Intention = judgeInfo.MapData.Intention;
		sentry.StartPositionX = judgeInfo.MapData.StartPositionX;
		sentry.StartPositionY = judgeInfo.MapData.StartPositionY;
		memcpy(sentry.DeltaX, judgeInfo.MapData.DeltaX, sizeof(sentry.DeltaX));
		memcpy(sentry.DeltaY, judgeInfo.MapData.DeltaY, sizeof(sentry.DeltaY));
		sentry.SenderId = judgeInfo.MapData.SenderId;

		msgpipe::Publish(sentry, K_NO_WAIT);
		break;
	}

	// 0x0311 自定义客户端 -> 机器人（图传）
	case static_cast<uint16_t>(protocol::rx::RefereeRxID::CustomClientData): {
		memcpy(
		    &judgeInfo.CustomClientData, pkg.Payload.data(),
		    static_cast<size_t>(protocol::rx::RefereeRxLength::CustomClientData)
		);

		memcpy(customClient.CustomClientData, judgeInfo.CustomClientData.Data, sizeof(customClient.CustomClientData));

		msgpipe::Publish(customClient, K_NO_WAIT);
		break;
	}
	}
}

} // namespace internal
} // namespace breeze::referee
