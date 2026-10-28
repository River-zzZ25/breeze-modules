/**
 * @file referee.h
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 裁判系统协议. 适用于2026赛季, 2.0.0协议.
 * @version 26.2.0
 * @date 2026-10-17
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstdint>

namespace breeze::referee::protocol
{

constexpr uint8_t SOF = 0xa5;

#pragma pack(push, 1)
/**
 * @brief 裁判系统帧头
 *
 */
struct FrameHeader
{
	uint8_t Sof;
	uint16_t DataLength;
	uint8_t Seq;
	uint8_t Crc8;
};
#pragma pack(pop)
static_assert(sizeof(FrameHeader) == 5, "referee frame header size mismatch");

namespace rx
{
// 命令码
enum class RefereeRxID : uint16_t
{
	GameStatus = 0x0001,
	GameRobotHp = 0x0003,
	EventData = 0x0101,
	DartInfo = 0x0105,
	RobotStatus = 0x0201,
	PowerHeatData = 0x0202,
	RobotPos = 0x0203,
	Buff = 0x0204,
	HurtData = 0x0206,
	ShootData = 0x0207,
	ProjectileAllowance = 0x0208,
	RfidStatus = 0x0209,
	GroundRobotPos = 0x020B,
	SentryInfo = 0x020D,
	CustomControllerData = 0x0302,
	MapCommand = 0x0303,
	MapData = 0x0307,
	CustomInfo = 0x0308,
	CustomClientData = 0x0311
};

// 结构体长度
enum class RefereeRxLength : uint8_t
{
	GameStatus = 11,
	GameRobotHp = 20,
	EventData = 4,
	DartInfo = 3,
	RobotStatus = 17,
	PowerHeatData = 14,
	RobotPos = 12,
	Buff = 8,
	HurtData = 1,
	ShootData = 7,
	ProjectileAllowance = 8,
	RfidStatus = 5,
	GroundRobotPos = 40,
	SentryInfo = 14,
	CustomControllerData = 30,
	MapCommand = 12,
	MapData = 105,
	CustomInfo = 34,
	CustomClientData = 30
};

/*---------------------结构体-----------------------*/
/* -------------------- 0x0001 -------------------- */
/**
 * @brief  比赛状态数据，固定以 1 Hz 频率发送
 * @note   0x0001
 */
typedef struct __attribute__((packed))
{
	uint8_t GameType : 4;     /* bit 0-3：比赛类型
	                           1：RoboMaster 机甲大师超级对抗赛
	                           2：RoboMaster 机甲大师高校单项赛
	                           3：ICRA RoboMaster 高校人工智能挑战赛
	                           4：RoboMaster 机甲大师高校联盟赛 3V3 对抗
	                           5：RoboMaster 机甲大师高校联盟赛步兵对抗 */
	uint8_t GameProgress : 4; /* bit 4-7：当前比赛阶段
	                           0：未开始比赛
	                           1：准备阶段
	                           2：十五秒裁判系统自检阶段
	                           3：五秒倒计时
	                           4：比赛中
	                           5：比赛结算中 */
	uint16_t StageRemainTime; /* 当前阶段剩余时间，单位：秒 */
	uint64_t SyncTimeStamp;   /* UNIX 时间，当机器人正确连接到裁判系统的 NTP 服务器后生效 */
} GameStatus;

/* -------------------- 0x0003 -------------------- */
/**
 * @brief  机器人血量数据，固定以 3 Hz 频率发送
 * @note   0x0003
 */
typedef struct __attribute__((packed))
{
	uint16_t Ally1RobotHp;
	uint16_t Ally2RobotHp;
	uint16_t Ally3RobotHp;
	uint16_t Ally4RobotHp;

	int16_t DamageDifference; /* 己方总伤害 - 对方总伤害 */

	uint16_t Ally7RobotHp;
	uint16_t AllyOutpostHp;
	uint16_t AllyBaseHp;

	uint16_t EnemyOutpostHp;
	uint16_t EnemyBaseHp;
} GameRobotHp;

/* -------------------- 0x0101 -------------------- */
/**
 * @brief  场地事件数据，固定以 1 Hz 频率发送
 * @note   0x0101
 *
 *  EventData 位域定义（bit0 为最低位）：
 *  bit 0     己方补给区占领状态                   1=已占领
 *  bit 1     保留位
 *  bit 2     己方补给区占领状态（仅 RMUL）        1=已占领
 *  bit 3-4   己方小能量机关激活状态               0=未激活 1=已激活 2=正在激活
 *  bit 5-6   己方大能量机关激活状态               0=未激活 1=已激活 2=正在激活
 *  bit 7-8   己方中央高地占领状态                 1=己方 2=对方
 *  bit 9-10  己方梯形高地占领状态                 1=已占领
 *  bit11-19  对方飞镖最后一次击中己方前哨站/基地时间 0-420 s
 *  bit20-22  对方飞镖最后一次击中目标             0=无 1=前哨站 2=基地固定 3=基地随机固定 4=基地随机移动 5=基地末端移动
 *  bit23-24  中心增益点占领状态（仅 RMUL）        0=无 1=己方 2=对方 3=双方
 *  bit25-26  己方堡垒增益点占领状态               0=无 1=己方 2=对方 3=双方
 *  bit27-28  己方前哨站增益点占领状态             0=无 1=己方 2=对方
 *  bit29     己方基地增益点占领状态               1=已占领
 *  bit30-31  保留
 */
typedef struct __attribute__((packed))
{
	uint32_t EventData;
} EventData;

/* -------------------- 0x0105 -------------------- */
/**
 * @brief  飞镖发射相关数据，固定以 1 Hz 频率发送
 * @note   0x0105
 */
typedef struct __attribute__((packed))
{
	uint8_t DartRemainingTime; /* 己方飞镖发射剩余时间，单位：秒 */
	uint16_t DartInfo;         /* bit 0-2：最近一次己方飞镖击中目标
	                              bit 3-5：对方被击中目标累计次数
	                              bit 6-8：飞镖当前选定目标
	                              bit 9-15：保留 */
} DartInfo;

/* -------------------- 0x0201 -------------------- */
/**
 * @brief  机器人性能体系数据，固定以 10 Hz 频率发送
 * @note   0x0201
 */
typedef struct __attribute__((packed))
{
	uint8_t RobotId;
	uint8_t RobotLevel;
	uint16_t CurrentHp;
	uint16_t MaximumHp;
	uint16_t ShooterBarrelCoolingValue;
	uint16_t ShooterBarrelHeatLimit;
	uint16_t ChassisPowerLimit;

	float BulletSpeedLimit;

	uint8_t PowerManagementGimbalOutput : 1;
	uint8_t PowerManagementChassisOutput : 1;
	uint8_t PowerManagementShooterOutput : 1;
} RobotStatus;

/* -------------------- 0x0202 -------------------- */
/**
 * @brief  实时底盘缓冲能量和射击热量数据，固定以 10 Hz 频率发送
 * @note   0x0202
 */
typedef struct __attribute__((packed))
{
	uint16_t Reserved1;
	uint16_t Reserved2;
	float Reserved3;
	uint16_t BufferEnergy;          /* 缓冲能量，单位：J */
	uint16_t Shooter17mmBarrelHeat; /* 17 mm 发射机构热量 */
	uint16_t Shooter42mmBarrelHeat; /* 42 mm 发射机构热量 */
} PowerHeatData;

/* -------------------- 0x0203 -------------------- */
/**
 * @brief  机器人位置数据，固定以 1 Hz 频率发送
 * @note   0x0203
 */
typedef struct __attribute__((packed))
{
	float X;     /* 本机器人位置 X 坐标，单位：m */
	float Y;     /* 本机器人位置 Y 坐标，单位：m */
	float Angle; /* 本机器人测速模块朝向，单位：度，正北为 0° */
} RobotPos;

/* -------------------- 0x0204 -------------------- */
/**
 * @brief  机器人增益和底盘能量数据，固定以 3 Hz 频率发送
 * @note   0x0204
 */
typedef struct __attribute__((packed))
{
	uint8_t RecoveryBuff;      /* 回血增益（百分比） */
	uint16_t CoolingBuff;      /* 射击热量冷却增益（直接值） */
	uint8_t DefenceBuff;       /* 防御增益（百分比） */
	uint8_t VulnerabilityBuff; /* 负防御增益（百分比） */
	uint16_t AttackBuff;       /* 攻击增益（百分比） */
	uint8_t RemainingEnergy;   /* 机器人剩余能量比例标识，详见协议位域说明 */
} Buff;

/* -------------------- 0x0206 -------------------- */
/**
 * @brief  伤害状态数据，伤害发生后发送
 * @note   0x0206
 */
typedef struct __attribute__((packed))
{
	uint8_t ArmorId : 4;           /* 扣血装甲/测速模块 ID */
	uint8_t HpDeductionReason : 4; /* 血量变化类型，详见协议枚举 */
} HurtData;

/* -------------------- 0x0207 -------------------- */
/**
 * @brief  实时射击数据，弹丸发射后发送
 * @note   0x0207
 */
typedef struct __attribute__((packed))
{
	uint8_t BulletType;         /* 弹丸类型：bit1-17 mm，bit2-42 mm */
	uint8_t ShooterNumber;      /* 发射机构 ID：1-17 mm，3-42 mm */
	uint8_t LaunchingFrequency; /* 弹丸射速，单位：Hz */
	float InitialSpeed;         /* 弹丸初速度，单位：m/s */
} ShootData;

/* -------------------- 0x0208 -------------------- */
/**
 * @brief  允许发弹量与剩余金币数，固定以 10 Hz 频率发送
 * @note   0x0208
 */
typedef struct __attribute__((packed))
{
	uint16_t ProjectileAllowance17mm;     /* 17 mm 弹丸允许发弹量 */
	uint16_t ProjectileAllowance42mm;     /* 42 mm 弹丸允许发弹量 */
	uint16_t RemainingGoldCoin;           /* 剩余金币数量 */
	uint16_t ProjectileAllowanceFortress; /* 堡垒增益点储备 17 mm 弹丸允许发弹量 */
} ProjectileAllowance;

/* -------------------- 0x0209 -------------------- */
/**
 * @brief  机器人 RFID 模块状态，固定以 3 Hz 频率发送
 * @note   0x0209
 */
typedef struct __attribute__((packed))
{
	uint32_t RfidStatus; /* 位图，详见协议表 1-19 */
	uint8_t RfidStatus2; /* 扩展位图，bit0-5 见协议表 1-19 */
} RfidStatus;

/* -------------------- 0x020B -------------------- */
/**
 * @brief  地面机器人位置数据，固定以 1 Hz 频率发送
 * @note   0x020B
 */
typedef struct __attribute__((packed))
{
	float HeroX; /* 己方英雄机器人位置 X 轴坐标，单位：m */
	float HeroY;
	float EngineerX;
	float EngineerY;
	float Standard3X;
	float Standard3Y;
	float Standard4X;
	float Standard4Y;
	float Reserved0;
	float Reserved1;
} GroundRobotPosition;

/* -------------------- 0x020D -------------------- */
/**
 * @brief  哨兵自主决策信息同步，固定以 1 Hz 频率发送
 * @note   0x020D
 */
typedef struct __attribute__((packed))
{
	uint32_t SentryInfo;
	uint16_t SentryInfo2;
	uint64_t SentryInfo3;
} SentryInfo;

/* -------------------- 0x0302 -------------------- */
/**
 * @brief  自定义控制器与机器人交互数据，发送方触发发送，频率上限 30 Hz，图传链路
 * @note   0x0302
 */
typedef struct __attribute__((packed))
{
	uint8_t Data[30]; /* 自定义数据 */
} CustomControllerData;

/* -------------------- 0x0303 -------------------- */
/**
 * @brief  选手端小地图交互数据，选手端触发发送
 * @note   0x0303
 */
typedef struct __attribute__((packed))
{
	float TargetPositionX; /* 目标位置 X 坐标，单位：m */
	float TargetPositionY; /* 目标位置 Y 坐标，单位：m */
	uint8_t CmdKeyboard;   /* 云台手按下的键盘按键通用键值，无按键为 0 */
	uint8_t TargetRobotId; /* 对方机器人 ID，发送坐标时为 0 */
	uint16_t CmdSource;    /* 信息来源 ID，详见协议附录 */
} MapCommand;

/* -------------------- 0x0307 -------------------- */
/**
 * @brief  选手端小地图接收路径数据，频率上限 1 Hz
 * @note   0x0307
 */
typedef struct __attribute__((packed))
{
	uint8_t Intention;       /* 1：攻击，2：防守，3：移动 */
	uint16_t StartPositionX; /* 路径起点 X 坐标，单位：dm */
	uint16_t StartPositionY; /* 路径起点 Y 坐标，单位：dm */
	int8_t DeltaX[49];       /* 路径点 X 轴增量数组，单位：dm */
	int8_t DeltaY[49];       /* 路径点 Y 轴增量数组，单位：dm */
	uint16_t SenderId;       /* 发送者 ID */
} MapData;

/* -------------------- 0x0308 -------------------- */
/**
 * @brief  选手端小地图接收机器人数据，频率上限 3 Hz
 * @note   0x0308
 */
typedef struct __attribute__((packed))
{
	uint16_t SenderId;    /* 发送者 ID */
	uint16_t ReceiverId;  /* 接收者 ID */
	uint8_t UserData[30]; /* UTF-16 编码字符，最大 30 字节 */
} CustomInfo;

/* -------------------- 0x0311 -------------------- */
/**
 * @brief  自定义客户端发送给机器人的自定义指令，频率上限 75 Hz，图传链路
 * @note   0x0311
 */
typedef struct __attribute__((packed))
{
	uint8_t Data[30]; /* 自定义指令 */
} CustomClientData;

typedef struct
{
	GameStatus GameStatus;
	GameRobotHp GameRobotHp;
	EventData EventData;
	DartInfo DartInfo;
	RobotStatus RobotStatus;
	PowerHeatData PowerHeatData;
	RobotPos RobotPos;
	Buff Buff;
	HurtData HurtData;
	ShootData ShootData;
	ProjectileAllowance ProjectileAllowance;
	RfidStatus RfidStatus;
	GroundRobotPosition GroundRobotPos;
	SentryInfo SentryInfo;
	CustomControllerData CustomControllerData;
	MapCommand MapCommand;
	MapData MapData;
	CustomInfo CustomInfo;
	CustomClientData CustomClientData;

} JudgeInfo;

// TODO: 试图作为裁判系统模块串口的心跳
// typedef enum {
//     DEV_ONLINE = 0,
//     DEV_OFFLINE = 1
// }dev_work_state_t;

// typedef struct
// {
//     uint16_t offline_cnt_max;
//     dev_work_state_t status;
//     uint16_t offline_cnt;
// }judge_status_t;
} // namespace rx
} // namespace breeze::referee::protocol
