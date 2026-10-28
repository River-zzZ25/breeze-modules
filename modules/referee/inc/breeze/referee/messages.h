/**
 * @file messages.h
 * @author vix_hentx (w1084349470@outlook.com), river_zzz
 * @brief 裁判系统模块bus消息
 * @version 0.1
 * @date 2026-10-16
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstdint>

namespace breeze::referee
{

#pragma region Rx Messages

// 标有 *图传* 的为图传链路数据，其他数据默认走常规链路
// 比赛状况
struct GameStateInfo
{
	// 0x0001 比赛状态
	uint8_t GameType;         /* 比赛类型
	                              1：RoboMaster 机甲大师超级对抗赛
	                              2：RoboMaster 机甲大师高校单项赛
	                              3：ICRA RoboMaster 高校人工智能挑战赛
	                              4：RoboMaster 机甲大师高校联盟赛 3V3 对抗
	                              5：RoboMaster 机甲大师高校联盟赛步兵对抗 */
	uint8_t GameProgress;     /* 当前比赛阶段
	                              0：未开始比赛
	                              1：准备阶段
	                              2：十五秒裁判系统自检阶段
	                              3：五秒倒计时
	                              4：比赛中
	                              5：比赛结算中 */
	uint16_t StageRemainTime; // 当前阶段剩余时间 单位：秒

	// 0x0003 机器人血量
	uint16_t AllyHeroHp;      // 己方英雄机器人血量
	uint16_t AllyEngineerHp;  // 己方工程机器人血量
	uint16_t AllyInfantry3Hp; // 己方3号步兵机器人血量
	uint16_t AllyInfantry4Hp; // 己方4号步兵机器人血量
	uint16_t AllySentryHp;    // 己方哨兵机器人血量
	uint16_t AllyOutpostHp;   // 己方前哨站血量
	uint16_t AllyBaseHp;      // 己方基地血量
	uint16_t EnemyOutpostHp;  // 敌方前哨站血量
	uint16_t EnemyBaseHp;     // 敌方基地血量

	// 0x0101 场地事件
	bool AllySupplyOcc;                    // 己方补给区的占领状态 1=已占领
	uint8_t AllySmallEnergyMechanismState; // 己方小能量机关激活状态 0=未激活 1=已激活 2=正在激活
	uint8_t AllyBigEnergyMechanismState;   // 己方大能量机关激活状态 0=未激活 1=已激活 2=正在激活
	uint8_t AllyCenterHighlandOcc;         // 己方中央高地占领状态 1=己方 2=对方
	bool AllyTrapezoidHighlandOcc;         // 己方梯形高地占领状态 1=已占领
	uint8_t EnemyDartLastHit;              /* 对方飞镖最后一次击中目标
	                                             0：无
	                                             1：前哨站
	                                             2：基地固定目标
	                                             3：基地随机固定目标
	                                             4：基地随机移动目标
	                                             5：基地末端移动目标 */
	uint8_t AllyFortBuffOcc;               // 己方堡垒增益点占领状态 0=无 1=己方 2=对方 3=双方
	uint8_t AllyOutpostBuffOcc;            // 己方前哨站增益点占领状态 0=无 1=己方 2=对方
	bool AllyBaseBuffOcc;                  // 己方基地增益点占领状态 1=已占领

	// 仅 RMUL 适用
	bool AllySupplyOccRmul;        // 己方补给区的占领状态，1 为已占领（仅 RMUL 适用）
	uint8_t AllyCenterBuffOccRmul; /* 中心增益点的占领状态（仅 RMUL 适用）
	                                      0 为未被占领
	                                      1 为被己方占领
	                                      2 为被对方占领
	                                      3 为被双方占领 */

	// 0x0105 飞镖发射
	uint8_t AllyDartRemainingTime; // 己方飞镖发射剩余时间 单位：秒
	uint8_t AllyDartLastHitTarget; /* 己方飞镖最后一次击中目标
	                                      0：无
	                                      1：前哨站
	                                      2：基地固定目标
	                                      3：基地随机固定目标
	                                      4：基地随机移动目标
	                                      5：基地末端移动目标 */
	uint8_t LastTargetHitTimes;    // 对方最近被击中的目标累计被击中计次数
	uint8_t LastTargetHitType;     /* 飞镖此时选定的击打目标
	                                     0：未选定/前哨站
	                                     1：基地固定目标
	                                     2：基地随机固定目标
	                                     3：基地随机移动目标
	                                     4：基地末端移动目标 */
};

// 机器人整体状态
struct RobotStateInfo
{
	// 0x0201 机器人性能体系数据
	uint8_t RobotId;    // 本机器人ID
	uint8_t RobotLevel; // 机器人等级
	uint16_t CurrentHp; // 机器人当前血量
	uint16_t MaximumHp; // 机器人血量上限

	// 0x0203 机器人位置数据
	float X;     // 本机器人位置 X 坐标，单位：m
	float Y;     // 本机器人位置 Y 坐标，单位：m
	float Angle; // 本机器人测速模块朝向，单位：度，正北为 0°

	// 0x0204 机器人增益和底盘能量数据
	uint8_t RecoveryBuff;    // 回血增益（百分比）
	uint8_t RemainingEnergy; /* 机器人剩余能量值反馈
	                             机器人初始能量视为 100%
	                             bit 0：在剩余能量≥125%时为 1，其余情况为 0
	                             bit 1：在剩余能量≥100%时为 1，其余情况为 0
	                             bit 2：在剩余能量≥50%时为 1，其余情况为 0
	                             bit 3：在剩余能量≥30%时为 1，其余情况为 0
	                             bit 4：在剩余能量≥15%时为 1，其余情况为 0
	                             bit 5：在剩余能量≥5%时为 1，其余情况为 0
	                             bit 6：在剩余能量≥1%时为 1，其余情况为 0 */

	// 0x0206 伤害状态数据
	uint8_t ArmorId;           // 扣血装甲/测速模块 ID
	uint8_t HpDeductionReason; // 血量变化类型，详见协议枚举

	// 0x0208 允许发弹量与剩余金币
	uint16_t RemainingGoldCoin; // 剩余金币数量

	// 0x0209 机器人 RFID 模块状态
	bool AllyBaseBuff;               // 己方基地增益点 0=未检测到 1=已检测到（下同）
	bool AllyCentralHighlandBuff;    // 己方中央高地增益点
	bool EnemyCentralHighlandBuff;   // 对方中央高地增益点
	bool AllyTrapezoidHighlandBuff;  // 己方梯形高地增益点
	bool EnemyTrapezoidHighlandBuff; // 对方梯形高地增益点

	bool AllyFlySlopeBefore;  // 己方地形跨越增益点（飞坡）（靠近己方一侧飞坡前）
	bool AllyFlySlopeAfter;   // 己方地形跨越增益点（飞坡）（靠近己方一侧飞坡后）
	bool EnemyFlySlopeBefore; // 对方地形跨越增益点（飞坡）（靠近对方一侧飞坡前）
	bool EnemyFlySlopeAfter;  // 对方地形跨越增益点（飞坡）（靠近对方一侧飞坡后）

	bool AllyCentralHighlandBelow;  // 己方地形跨越增益点（中央高地下方）
	bool AllyCentralHighlandAbove;  // 己方地形跨越增益点（中央高地上方）
	bool EnemyCentralHighlandBelow; // 对方地形跨越增益点（中央高地下方）
	bool EnemyCentralHighlandAbove; // 对方地形跨越增益点（中央高地上方）

	bool AllyHighwayBelow;  // 己方地形跨越增益点（公路下方）
	bool AllyHighwayAbove;  // 己方地形跨越增益点（公路上方）
	bool EnemyHighwayBelow; // 对方地形跨越增益点（公路下方）
	bool EnemyHighwayAbove; // 对方地形跨越增益点（公路上方）

	bool AllyFortBuff;        // 己方堡垒增益点
	bool AllyOutpostBuff;     // 己方前哨站增益点
	bool AllySupplyNoOverlap; // 己方与资源区不重叠的补给区 / RMUL 补给区
	bool AllySupplyOverlap;   // 己方与资源区重叠的补给区
	bool AllyAssemblyBuff;    // 己方装配增益点
	bool EnemyAssemblyBuff;   // 对方装配增益点
	bool EnemyFortBuff;       // 对方堡垒增益点
	bool EnemyOutpostBuff;    // 对方前哨站增益点

	bool AllyTunnelHighwayBelow;  // 己方地形跨越增益点（隧道）（靠近己方一侧公路区下方）
	bool AllyTunnelHighwayMid;    // 己方地形跨越增益点（隧道）（靠近己方一侧公路区中间）
	bool AllyTunnelHighwayAbove;  // 己方地形跨越增益点（隧道）（靠近己方一侧公路区上方）
	bool AllyTunnelHighlandBelow; // 己方地形跨越增益点（隧道）（靠近己方梯形高地较低处）
	bool AllyTunnelHighlandMid;   // 己方地形跨越增益点（隧道）（靠近己方梯形高地较中间）
	bool AllyTunnelHighlandAbove; // 己方地形跨越增益点（隧道）（靠近己方梯形高地较高处）

	bool EnemyTunnelHighwayBelow;  // 对方地形跨越增益点（隧道）（靠近对方公路一侧下方）
	bool EnemyTunnelHighwayMid;    // 对方地形跨越增益点（隧道）（靠近对方公路一侧中间）
	bool EnemyTunnelHighwayAbove;  // 对方地形跨越增益点（隧道）（靠近对方公路一侧上方）
	bool EnemyTunnelHighlandBelow; // 对方地形跨越增益点（隧道）（靠近对方梯形高地较低处）
	bool EnemyTunnelHighlandMid;   // 对方地形跨越增益点（隧道）（靠近对方梯形高地较中间）
	bool EnemyTunnelHighlandAbove; // 对方地形跨越增益点（隧道）（靠近对方梯形高地较高处）

	// 仅 RMUL 适用
	bool CenterBuffRmul; // 中心增益点（仅 RMUL 适用）
};

// 底盘状态
struct ChassisStateInfo
{
	// 0x0201 机器人性能体系数据
	uint16_t ChassisPowerLimit; // 机器人底盘功率上限

	// 0x0202 实时底盘缓冲能量和射击热量数据
	uint16_t BufferEnergy; // 缓冲能量 单位：J
};

// 发射机构状态
struct ShooterStateInfo
{
	// 0x0201 机器人性能体系数据
	uint16_t ShooterBarrelCoolingValue; // 机器人射击热量每秒冷却值
	uint16_t ShooterBarrelHeatLimit;    // 机器人射击热量上限
	float BulletSpeedLimit;             // 机器人射击初速度上限

	// 0x0202 实时底盘缓冲能量和射击热量数据
	uint16_t Shooter17mmBarrelHeat; // 17 mm 发射机构热量
	uint16_t Shooter42mmBarrelHeat; // 42 mm 发射机构热量

	// 0x0207 实时射击数据
	uint8_t LaunchingFrequency; // 弹丸射速，单位：Hz
	float InitialSpeed;         // 弹丸初速度，单位：m/s

	// 0x0208 允许发弹量与剩余金币
	uint16_t ProjectileAllowance17mm; // 17 mm 弹丸允许发弹量
	uint16_t ProjectileAllowance42mm; // 42 mm 弹丸允许发弹量
};

// 哨兵专属信息
struct SentryOnlyInfo
{
	// 0x020B 地面机器人位置数据
	float AllyHeroX;      // 己方英雄机器人位置 X 轴坐标，单位：m
	float AllyHeroY;      // 己方英雄机器人位置 Y 轴坐标，单位：m
	float AllyEngineerX;  // 己方工程机器人位置 X 轴坐标，单位：m
	float AllyEngineerY;  // 己方工程机器人位置 Y 轴坐标，单位：m
	float AllyInfantry3X; // 己方 3 号步兵机器人位置 X 轴坐标，单位：m
	float AllyInfantry3Y; // 己方 3 号步兵机器人位置 Y 轴坐标，单位：m
	float AllyInfantry4X; // 己方 4 号步兵机器人位置 X 轴坐标，单位：m
	float AllyInfantry4Y; // 己方 4 号步兵机器人位置 Y 轴坐标，单位：m

	// 0x020D 哨兵自主决策信息同步
	uint16_t ExchangeProjectileAllowance;           // 除远程兑换外成功兑换的允许发弹量，开局为 0
	uint8_t RemoteExchangeProjectileAllowanceTimes; // 成功远程兑换允许发弹量的次数
	uint8_t RemoteExchangeHpTimes;                  // 成功远程兑换血量的次数
	bool FreeReviveAvailable;                       // 当前是否可以确认免费复活 0=否 1=是
	bool InstantReviveAvailable;                    // 当前是否可以兑换立即复活 0=否 1=是
	uint16_t InstantReviveCost;                     // 兑换立即复活需要的金币数
	bool OutOfCombat;                               // 当前是否处于脱战状态 0=否 1=是
	uint16_t TeamProjectileAllowance17mmLeft;       // 队伍 17mm 允许发弹量的剩余可兑换数
	uint8_t Posture;                                // 当前姿态 1=进攻 2=防御 3=移动
	uint8_t EnergyMechanismActivateAllowance;       // 己方能量机关是否能够进入正在激活状态 1=可激活
	uint8_t EnhancedPosture;                        // 当前姿态是否为强化姿态
	uint8_t AttackBeforeWeakTimeLeft;               // 进攻姿态弱化前剩余可持续时长 单位：秒
	uint8_t DefenseBeforeWeakTimeLeft;              // 防御姿态弱化前剩余可持续时长 单位：秒
	uint8_t MoveBeforeWeakTimeLeft;                 // 移动姿态弱化前剩余可持续时长 单位：秒
	uint8_t EnhancedAttackTimeLeft;                 // 强化进攻姿态剩余可持续时长 单位：秒
	uint8_t EnhancedDefenseTimeLeft;                // 强化防御姿态剩余可持续时长 单位：秒
	uint8_t EnhancedMoveTimeLeft;                   // 强化移动姿态剩余可持续时长 单位：秒

	// 0x0307 选手端小地图接收路径数据
	uint8_t Intention;       // 1=到目标点攻击 2=到目标点防守 3=移动到目标点
	uint16_t StartPositionX; // 路径起点 X 轴坐标
	uint16_t StartPositionY; // 路径起点 Y 轴坐标
	int8_t DeltaX[49];       // 路径点 X 轴增量数组
	int8_t DeltaY[49];       // 路径点 Y 轴增量数组
	uint16_t SenderId;       // 发送者 ID
};

// 自定义控制器信息
struct CustomControllerRxInfo
{
	// 0x0302 自定义控制器 -> 机器人（图传）
	uint8_t CustomControllerData[30]; // 自定义数据
};

// 自定义客户端信息
struct CustomClientRxInfo
{
	// 0x0311 自定义客户端 -> 机器人（图传）
	uint8_t CustomClientData[30]; // 自定义数据
};

// 选手端信息
struct PlayerClientRxInfo
{
	// 0x0303 选手端 -> 机器人（图传）
	float TargetPositionX; // 目标位置 X 轴坐标
	float TargetPositionY; // 目标位置 Y 轴坐标
	uint8_t CmdKeyboard;   // 云台手按下的键盘按键通用键值
	uint8_t TargetRobotId; // 对方机器人 ID
	uint16_t CmdSource;    // 信息来源 ID
};

#pragma endregion

#pragma region Tx Messages

// 自定义控制器信息
struct CustomControllerTxInfo
{
	// 0x0309 机器人 -> 自定义控制器（图传）
	uint8_t CustomControllerData[30]; // 自定义数据
};

// 自定义客户端信息
struct CustomClientTxInfo
{
	// 0x0310 机器人 -> 自定义客户端（图传）
	uint8_t DataTx[300]; // 自定义数据
};

// 选手端信息
struct PlayerClientTxInfo
{
	// 0x0308 机器人 -> 选手端（图传）
	uint16_t SenderId;    // 发送者的 ID
	uint16_t ReceiverId;  // 接收者的 ID
	uint8_t UserData[30]; /* 字符
	                          以 utf-16 格式编码发送，支持显示中文
	                          编码发送时请注意数据的大小端问题 */
};

#pragma endregion

} // namespace breeze::referee
