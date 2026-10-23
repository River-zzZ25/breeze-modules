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

constexpr uint8_t sof = 0xa5;

#pragma pack(push,1)
/**
 * @brief 裁判系统帧头
 * 
 */
struct frame_header
{
    uint8_t sof;
    uint16_t data_length;
    uint8_t seq;
    uint8_t crc8;
};
#pragma pack(pop)
static_assert(sizeof(frame_header) == 5, "referee frame header size mismatch");

namespace rx
{
    // 命令码
    enum class RefereeRxID : uint16_t
    {
        game_status = 0x0001,
        game_robot_hp = 0x0003,
        event_data = 0x0101,
        dart_info = 0x0105,
        robot_status = 0x0201,
        power_heat_data = 0x0202,
        robot_pos = 0x0203,
        buff = 0x0204,
        hurt_data = 0x0206,
        shoot_data = 0x0207,
        projectile_allowance = 0x0208,
        rfid_status = 0x0209,
        ground_robot_pos = 0x020B,
        sentry_info = 0x020D,
        custom_controller_data = 0x0302,
        map_command = 0x0303,
        map_data = 0x0307,
        custom_info = 0x0308,
        custom_client_data = 0x0311
    };

    // 结构体长度
    enum class RefereeRxlength : uint8_t
    {
        game_status = 11,
        game_robot_hp = 20,
        event_data = 4,
        dart_info = 3,
        robot_status = 17,
        power_heat_data = 14,
        robot_pos = 12,
        buff = 8,
        hurt_data = 1,
        shoot_data = 7,
        projectile_allowance = 8,
        rfid_status = 5,
        ground_robot_pos = 40,
        sentry_info = 14,
        custom_controller_data = 30,
        map_command = 12,
        map_data = 105,
        custom_info = 34,
        custom_client_data = 30
    };

    /*---------------------结构体-----------------------*/
    /* -------------------- 0x0001 -------------------- */
    /**
     * @brief  比赛状态数据，固定以 1 Hz 频率发送
     * @note   0x0001
     */
    typedef struct __attribute__((packed))
    {
        uint8_t  game_type     : 4; /* bit 0-3：比赛类型
                                    1：RoboMaster 机甲大师超级对抗赛
                                    2：RoboMaster 机甲大师高校单项赛
                                    3：ICRA RoboMaster 高校人工智能挑战赛
                                    4：RoboMaster 机甲大师高校联盟赛 3V3 对抗
                                    5：RoboMaster 机甲大师高校联盟赛步兵对抗 */
        uint8_t  game_progress : 4; /* bit 4-7：当前比赛阶段
                                    0：未开始比赛
                                    1：准备阶段
                                    2：十五秒裁判系统自检阶段
                                    3：五秒倒计时
                                    4：比赛中
                                    5：比赛结算中 */
        uint16_t stage_remain_time; /* 当前阶段剩余时间，单位：秒 */
        uint64_t sync_time_stamp;     /* UNIX 时间，当机器人正确连接到裁判系统的 NTP 服务器后生效 */
    } game_status_t;

    /* -------------------- 0x0003 -------------------- */
    /**
     * @brief  机器人血量数据，固定以 3 Hz 频率发送
     * @note   0x0003
     */
    typedef struct __attribute__((packed))
    {
        uint16_t ally_1_robot_hp;
        uint16_t ally_2_robot_hp;
        uint16_t ally_3_robot_hp;
        uint16_t ally_4_robot_hp;

        int16_t damage_difference;    /* 己方总伤害 - 对方总伤害 */

        uint16_t ally_7_robot_hp;
        uint16_t ally_outpost_hp;
        uint16_t ally_base_hp;

        uint16_t enemy_outpost_hp;
        uint16_t enemy_base_hp;
    } game_robot_hp_t;


    /* -------------------- 0x0101 -------------------- */
    /**
     * @brief  场地事件数据，固定以 1 Hz 频率发送
     * @note   0x0101
     *
     *  event_data 位域定义（bit0 为最低位）：
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
        uint32_t event_data;
    } event_data_t;

    /* -------------------- 0x0105 -------------------- */
    /**
     * @brief  飞镖发射相关数据，固定以 1 Hz 频率发送
     * @note   0x0105
     */
    typedef struct __attribute__((packed))
    {
        uint8_t  dart_remaining_time; /* 己方飞镖发射剩余时间，单位：秒 */
        uint16_t dart_info;           /* bit 0-2：最近一次己方飞镖击中目标
                                        bit 3-5：对方被击中目标累计次数
                                        bit 6-8：飞镖当前选定目标
                                        bit 9-15：保留 */
    } dart_info_t;

    /* -------------------- 0x0201 -------------------- */
    /**
     * @brief  机器人性能体系数据，固定以 10 Hz 频率发送
     * @note   0x0201
     */
    typedef struct __attribute__((packed))
    {
        uint8_t  robot_id;
        uint8_t  robot_level;
        uint16_t current_hp;
        uint16_t maximum_hp;
        uint16_t shooter_barrel_cooling_value;
        uint16_t shooter_barrel_heat_limit;
        uint16_t chassis_power_limit;

        float bullet_speed_limit;

        uint8_t power_management_gimbal_output  : 1;
        uint8_t power_management_chassis_output : 1;
        uint8_t power_management_shooter_output : 1;
    } robot_status_t;


    /* -------------------- 0x0202 -------------------- */
    /**
     * @brief  实时底盘缓冲能量和射击热量数据，固定以 10 Hz 频率发送
     * @note   0x0202
     */
    typedef struct __attribute__((packed))
    {
        uint16_t reserved_1;
        uint16_t reserved_2;
        float    reserved_3;
        uint16_t buffer_energy;             /* 缓冲能量，单位：J */
        uint16_t shooter_17mm_barrel_heat;   /* 17 mm 发射机构热量 */
        uint16_t shooter_42mm_barrel_heat;   /* 42 mm 发射机构热量 */
    } power_heat_data_t;

    /* -------------------- 0x0203 -------------------- */
    /**
     * @brief  机器人位置数据，固定以 1 Hz 频率发送
     * @note   0x0203
     */
    typedef struct __attribute__((packed))
    {
        float x;     /* 本机器人位置 x 坐标，单位：m */
        float y;     /* 本机器人位置 y 坐标，单位：m */
        float angle; /* 本机器人测速模块朝向，单位：度，正北为 0° */
    } robot_pos_t;

    /* -------------------- 0x0204 -------------------- */
    /**
     * @brief  机器人增益和底盘能量数据，固定以 3 Hz 频率发送
     * @note   0x0204
     */
    typedef struct __attribute__((packed))
    {
        uint8_t  recovery_buff;      /* 回血增益（百分比） */
        uint16_t cooling_buff;       /* 射击热量冷却增益（直接值） */
        uint8_t  defence_buff;       /* 防御增益（百分比） */
        uint8_t  vulnerability_buff; /* 负防御增益（百分比） */
        uint16_t attack_buff;        /* 攻击增益（百分比） */
        uint8_t  remaining_energy;   /* 机器人剩余能量比例标识，详见协议位域说明 */
    } buff_t;

    /* -------------------- 0x0206 -------------------- */
    /**
     * @brief  伤害状态数据，伤害发生后发送
     * @note   0x0206
     */
    typedef struct __attribute__((packed))
    {
        uint8_t armor_id : 4;            /* 扣血装甲/测速模块 ID */
        uint8_t hp_deduction_reason : 4; /* 血量变化类型，详见协议枚举 */
    } hurt_data_t;

    /* -------------------- 0x0207 -------------------- */
    /**
     * @brief  实时射击数据，弹丸发射后发送
     * @note   0x0207
     */
    typedef struct __attribute__((packed))
    {
        uint8_t bullet_type;       /* 弹丸类型：bit1-17 mm，bit2-42 mm */
        uint8_t shooter_number;    /* 发射机构 ID：1-17 mm，3-42 mm */
        uint8_t launching_frequency; /* 弹丸射速，单位：Hz */
        float   initial_speed;     /* 弹丸初速度，单位：m/s */
    } shoot_data_t;

    /* -------------------- 0x0208 -------------------- */
    /**
     * @brief  允许发弹量与剩余金币数，固定以 10 Hz 频率发送
     * @note   0x0208
     */
    typedef struct __attribute__((packed))
    {
        uint16_t projectile_allowance_17mm;   /* 17 mm 弹丸允许发弹量 */
        uint16_t projectile_allowance_42mm;   /* 42 mm 弹丸允许发弹量 */
        uint16_t remaining_gold_coin;         /* 剩余金币数量 */
        uint16_t projectile_allowance_fortress; /* 堡垒增益点储备 17 mm 弹丸允许发弹量 */
    } projectile_allowance_t;

    /* -------------------- 0x0209 -------------------- */
    /**
     * @brief  机器人 RFID 模块状态，固定以 3 Hz 频率发送
     * @note   0x0209
     */
    typedef struct __attribute__((packed))
    {
        uint32_t rfid_status;   /* 位图，详见协议表 1-19 */
        uint8_t  rfid_status_2; /* 扩展位图，bit0-5 见协议表 1-19 */
    } rfid_status_t;

    /* -------------------- 0x020B -------------------- */
    /**
     * @brief  地面机器人位置数据，固定以 1 Hz 频率发送
     * @note   0x020B
     */
    typedef struct __attribute__((packed))
    {
        float hero_x;       /* 己方英雄机器人位置 x 轴坐标，单位：m */
        float hero_y;
        float engineer_x;
        float engineer_y;
        float standard_3_x;
        float standard_3_y;
        float standard_4_x;
        float standard_4_y;
        float reserved_0;
        float reserved_1;
    } ground_robot_position_t;

    /* -------------------- 0x020D -------------------- */
    /**
     * @brief  哨兵自主决策信息同步，固定以 1 Hz 频率发送
     * @note   0x020D
     */
    typedef struct __attribute__((packed))
    {
        uint32_t sentry_info;
        uint16_t sentry_info_2;
        uint64_t sentry_info_3;
    } sentry_info_t;

    /* -------------------- 0x0302 -------------------- */
    /**
     * @brief  自定义控制器与机器人交互数据，发送方触发发送，频率上限 30 Hz，图传链路
     * @note   0x0302
     */
    typedef struct __attribute__((packed))
    {
        uint8_t data[30]; /* 自定义数据 */
    } custom_controller_data_t;

    /* -------------------- 0x0303 -------------------- */
    /**
     * @brief  选手端小地图交互数据，选手端触发发送
     * @note   0x0303
     */
    typedef struct __attribute__((packed))
    {
        float    target_position_x; /* 目标位置 x 坐标，单位：m */
        float    target_position_y; /* 目标位置 y 坐标，单位：m */
        uint8_t  cmd_keyboard;      /* 云台手按下的键盘按键通用键值，无按键为 0 */
        uint8_t  target_robot_id;   /* 对方机器人 ID，发送坐标时为 0 */
        uint16_t cmd_source;        /* 信息来源 ID，详见协议附录 */
    } map_command_t;

    /* -------------------- 0x0307 -------------------- */
    /**
     * @brief  选手端小地图接收路径数据，频率上限 1 Hz
     * @note   0x0307
     */
    typedef struct __attribute__((packed))
    {
        uint8_t  intention;         /* 1：攻击，2：防守，3：移动 */
        uint16_t start_position_x;  /* 路径起点 x 坐标，单位：dm */
        uint16_t start_position_y;  /* 路径起点 y 坐标，单位：dm */
        int8_t   delta_x[49];       /* 路径点 x 轴增量数组，单位：dm */
        int8_t   delta_y[49];       /* 路径点 y 轴增量数组，单位：dm */
        uint16_t sender_id;         /* 发送者 ID */
    } map_data_t;

    /* -------------------- 0x0308 -------------------- */
    /**
     * @brief  选手端小地图接收机器人数据，频率上限 3 Hz
     * @note   0x0308
     */
    typedef struct __attribute__((packed))
    {
        uint16_t sender_id;    /* 发送者 ID */
        uint16_t receiver_id;  /* 接收者 ID */
        uint8_t  user_data[30]; /* UTF-16 编码字符，最大 30 字节 */
    } custom_info_t;

    /* -------------------- 0x0311 -------------------- */
    /**
     * @brief  自定义客户端发送给机器人的自定义指令，频率上限 75 Hz，图传链路
     * @note   0x0311
     */
    typedef struct __attribute__((packed))
    {
        uint8_t data[30]; /* 自定义指令 */
    } custom_client_data_t;


    typedef struct{
        game_status_t              game_status;           
        game_robot_hp_t            game_robot_hp;        
        event_data_t               event_data;           
        dart_info_t                dart_info;            
        robot_status_t             robot_status;        
        power_heat_data_t          power_heat_data;       
        robot_pos_t                robot_pos;             
        buff_t                     buff;            
        hurt_data_t                hurt_data;           
        shoot_data_t               shoot_data;         
        projectile_allowance_t     projectile_allowance;
        rfid_status_t              rfid_status;         
        ground_robot_position_t    ground_robot_pos;
        sentry_info_t              sentry_info;
        custom_controller_data_t   custom_controller_data;
        map_command_t              map_command;       
        map_data_t                 map_data;           
        custom_info_t              custom_info;      
        custom_client_data_t       custom_client_data;

    }judge_info_t;

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
}
}