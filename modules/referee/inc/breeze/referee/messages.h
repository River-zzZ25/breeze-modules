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
struct game_state_info 
{
    // 0x0001 比赛状态
    uint8_t game_type;                          /* 比赛类型
                                                   1：RoboMaster 机甲大师超级对抗赛
                                                   2：RoboMaster 机甲大师高校单项赛
                                                   3：ICRA RoboMaster 高校人工智能挑战赛
                                                   4：RoboMaster 机甲大师高校联盟赛 3V3 对抗
                                                   5：RoboMaster 机甲大师高校联盟赛步兵对抗 */
    uint8_t game_progress;                      /* 当前比赛阶段
                                                   0：未开始比赛
                                                   1：准备阶段
                                                   2：十五秒裁判系统自检阶段
                                                   3：五秒倒计时
                                                   4：比赛中
                                                   5：比赛结算中 */
    uint16_t stage_remain_time;                 // 当前阶段剩余时间 单位：秒

    // 0x0003 机器人血量
    uint16_t ally_hero_hp;                      // 己方英雄机器人血量
    uint16_t ally_engineer_hp;                  // 己方工程机器人血量
    uint16_t ally_infantry_3_hp;                // 己方3号步兵机器人血量
    uint16_t ally_infantry_4_hp;                // 己方4号步兵机器人血量
    uint16_t ally_sentry_hp;                    // 己方哨兵机器人血量
    uint16_t ally_outpost_hp;                   // 己方前哨站血量
    uint16_t ally_base_hp;                      // 己方基地血量
    uint16_t enemy_outpost_hp;                  // 敌方前哨站血量
    uint16_t enemy_base_hp;                     // 敌方基地血量

    // 0x0101 场地事件
    bool ally_supply_occ ;                      // 己方补给区的占领状态 1=已占领
    uint8_t ally_small_energy_mechanism_state;  // 己方小能量机关激活状态 0=未激活 1=已激活 2=正在激活
    uint8_t ally_big_energy_mechanism_state;    // 己方大能量机关激活状态 0=未激活 1=已激活 2=正在激活
    uint8_t ally_center_highland_occ;           // 己方中央高地占领状态 1=己方 2=对方
    bool ally_trapezoid_highland_occ;           // 己方梯形高地占领状态 1=已占领
    uint8_t enemy_dart_last_hit;                /* 对方飞镖最后一次击中目标
                                                   0：无
                                                   1：前哨站
                                                   2：基地固定目标
                                                   3：基地随机固定目标
                                                   4：基地随机移动目标
                                                   5：基地末端移动目标 */
    uint8_t ally_fort_buff_occ;                 // 己方堡垒增益点占领状态 0=无 1=己方 2=对方 3=双方
    uint8_t ally_outpost_buff_occ;              // 己方前哨站增益点占领状态 0=无 1=己方 2=对方
    bool ally_base_buff_occ;                    // 己方基地增益点占领状态 1=已占领

    // 仅 RMUL 适用
    bool ally_supply_occ_rmul;                  // 己方补给区的占领状态，1 为已占领（仅 RMUL 适用）
    uint8_t ally_outpost_buff_occ_rmul;         /* 己方前哨站增益点的占领状态（仅 RMUL 适用）
                                                   0 为未被占领
                                                   1 为被己方占领
                                                   2 为被对方占领 */

    // 0x0105 飞镖发射
    uint8_t ally_dart_remaining_time;           // 己方飞镖发射剩余时间 单位：秒
    uint8_t ally_dart_last_hit_target;          /* 己方飞镖最后一次击中目标
                                                   0：无
                                                   1：前哨站
                                                   2：基地固定目标
                                                   3：基地随机固定目标
                                                   4：基地随机移动目标
                                                   5：基地末端移动目标 */
    uint8_t last_target_hit_times;              // 对方最近被击中的目标累计被击中计次数
    uint8_t last_target_hit_type;               /* 飞镖此时选定的击打目标
                                                   0：未选定/前哨站
                                                   1：基地固定目标
                                                   2：基地随机固定目标
                                                   3：基地随机移动目标
                                                   4：基地末端移动目标 */
};

// 机器人整体状态
struct robot_state_info
{
    // 0x0201 机器人性能体系数据
    uint8_t  robot_id;                      // 本机器人ID                      
    uint8_t  robot_level;                   // 机器人等级
    uint16_t current_HP;                    // 机器人当前血量
    uint16_t maximum_HP;                    // 机器人血量上限

    // 0x0203 机器人位置数据
    float x;                                // 本机器人位置 x 坐标，单位：m
    float y;                                // 本机器人位置 y 坐标，单位：m
    float angle;                            // 本机器人测速模块朝向，单位：度，正北为 0°

    // 0x0204 机器人增益和底盘能量数据
    uint8_t  recovery_buff;                 // 回血增益（百分比）
    uint8_t  remaining_energy;              /* 机器人剩余能量值反馈
                                               机器人初始能量视为 100%
                                               bit 0：在剩余能量≥125%时为 1，其余情况为 0
                                               bit 1：在剩余能量≥100%时为 1，其余情况为 0
                                               bit 2：在剩余能量≥50%时为 1，其余情况为 0
                                               bit 3：在剩余能量≥30%时为 1，其余情况为 0
                                               bit 4：在剩余能量≥15%时为 1，其余情况为 0
                                               bit 5：在剩余能量≥5%时为 1，其余情况为 0
                                               bit 6：在剩余能量≥1%时为 1，其余情况为 0 */

    // 0x0206 伤害状态数据
    uint8_t armor_id;                       // 扣血装甲/测速模块 ID
    uint8_t HP_deduction_reason;            // 血量变化类型，详见协议枚举

    // 0x0208 允许发弹量与剩余金币
    uint16_t remaining_gold_coin;           // 剩余金币数量
    
    // 0x0209 机器人 RFID 模块状态
    bool ally_base_buff;                    // 己方基地增益点 0=未检测到 1=已检测到（下同）
    bool ally_central_highland_buff;        // 己方中央高地增益点
    bool enemy_central_highland_buff;       // 对方中央高地增益点
    bool ally_trapezoid_highland_buff;      // 己方梯形高地增益点
    bool enemy_trapezoid_highland_buff;     // 对方梯形高地增益点

    bool ally_fly_slope_before;             // 己方地形跨越增益点（飞坡）（靠近己方一侧飞坡前）
    bool ally_fly_slope_after;              // 己方地形跨越增益点（飞坡）（靠近己方一侧飞坡后）
    bool enemy_fly_slope_before;            // 对方地形跨越增益点（飞坡）（靠近对方一侧飞坡前）
    bool enemy_fly_slope_after;             // 对方地形跨越增益点（飞坡）（靠近对方一侧飞坡后）

    bool ally_central_highland_below;       // 己方地形跨越增益点（中央高地下方）
    bool ally_central_highland_above;       // 己方地形跨越增益点（中央高地上方）
    bool enemy_central_highland_below;      // 对方地形跨越增益点（中央高地下方）
    bool enemy_central_highland_above;      // 对方地形跨越增益点（中央高地上方）

    bool ally_highway_below;                // 己方地形跨越增益点（公路下方）
    bool ally_highway_above;                // 己方地形跨越增益点（公路上方）
    bool enemy_highway_below;               // 对方地形跨越增益点（公路下方）
    bool enemy_highway_above;               // 对方地形跨越增益点（公路上方）

    bool ally_fort_buff;                    // 己方堡垒增益点
    bool ally_outpost_buff;                 // 己方前哨站增益点
    bool ally_supply_no_overlap;            // 己方与资源区不重叠的补给区 / RMUL 补给区
    bool ally_supply_overlap;               // 己方与资源区重叠的补给区
    bool ally_assembly_buff;                // 己方装配增益点
    bool enemy_assembly_buff;               // 对方装配增益点
    bool enemy_fort_buff;                   // 对方堡垒增益点
    bool enemy_outpost_buff;                // 对方前哨站增益点

    bool ally_tunnel_highway_below;         // 己方地形跨越增益点（隧道）（靠近己方一侧公路区下方）
    bool ally_tunnel_highway_mid;           // 己方地形跨越增益点（隧道）（靠近己方一侧公路区中间）
    bool ally_tunnel_highway_above;         // 己方地形跨越增益点（隧道）（靠近己方一侧公路区上方）
    bool ally_tunnel_highland_below;        // 己方地形跨越增益点（隧道）（靠近己方梯形高地较低处）
    bool ally_tunnel_highland_mid;          // 己方地形跨越增益点（隧道）（靠近己方梯形高地较中间）
    bool ally_tunnel_highland_above;        // 己方地形跨越增益点（隧道）（靠近己方梯形高地较高处）

    bool enemy_tunnel_highway_below;        // 对方地形跨越增益点（隧道）（靠近对方公路一侧下方）
    bool enemy_tunnel_highway_mid;          // 对方地形跨越增益点（隧道）（靠近对方公路一侧中间）
    bool enemy_tunnel_highway_above;        // 对方地形跨越增益点（隧道）（靠近对方公路一侧上方）
    bool enemy_tunnel_highland_below;       // 对方地形跨越增益点（隧道）（靠近对方梯形高地较低处）
    bool enemy_tunnel_highland_mid;         // 对方地形跨越增益点（隧道）（靠近对方梯形高地较中间）
    bool enemy_tunnel_highland_above;       // 对方地形跨越增益点（隧道）（靠近对方梯形高地较高处）

    // 仅 RMUL 适用
    bool center_buff_rmul;                  // 中心增益点（仅 RMUL 适用）

};

// 底盘状态
struct chassis_state_info
{
    // 0x0201 机器人性能体系数据
    uint16_t chassis_power_limit;           // 机器人底盘功率上限

    // 0x0202 实时底盘缓冲能量和射击热量数据
    uint16_t buffer_energy;                 // 缓冲能量 单位：J
};

// 发射机构状态
struct shooter_state_info
{
    // 0x0201 机器人性能体系数据
    uint16_t shooter_barrel_cooling_value;      // 机器人射击热量每秒冷却值
    uint16_t shooter_barrel_heat_limit;         // 机器人射击热量上限
    float bullet_speed_limit;                   // 机器人射击初速度上限

    // 0x0202 实时底盘缓冲能量和射击热量数据
    uint16_t shooter_17mm_barrel_heat;          // 17 mm 发射机构热量
    uint16_t shooter_42mm_barrel_heat;          // 42 mm 发射机构热量

    // 0x0207 实时射击数据
    uint8_t launching_frequency;                // 弹丸射速，单位：Hz
    float   initial_speed;                      // 弹丸初速度，单位：m/s

    // 0x0208 允许发弹量与剩余金币
    uint16_t projectile_allowance_17mm;         // 17 mm 弹丸允许发弹量
    uint16_t projectile_allowance_42mm;         // 42 mm 弹丸允许发弹量
};

// 哨兵专属信息
struct sentry_only_info
{
    // 0x020B 地面机器人位置数据
    float ally_hero_x;                                      // 己方英雄机器人位置 x 轴坐标，单位：m
    float ally_hero_y;                                      // 己方英雄机器人位置 y 轴坐标，单位：m
    float ally_engineer_x;                                  // 己方工程机器人位置 x 轴坐标，单位：m
    float ally_engineer_y;                                  // 己方工程机器人位置 y 轴坐标，单位：m
    float ally_infantry_3_x;                                // 己方 3 号步兵机器人位置 x 轴坐标，单位：m
    float ally_infantry_3_y;                                // 己方 3 号步兵机器人位置 y 轴坐标，单位：m
    float ally_infantry_4_x;                                // 己方 4 号步兵机器人位置 x 轴坐标，单位：m
    float ally_infantry_4_y;                                // 己方 4 号步兵机器人位置 y 轴坐标，单位：m

    // 0x020D 哨兵自主决策信息同步
    uint16_t exchange_projectile_allowance;                 // 除远程兑换外成功兑换的允许发弹量，开局为 0
    uint8_t remote_exchange_projectile_allowance_times;     // 成功远程兑换允许发弹量的次数
    uint8_t remote_exchange_hp_times;                       // 成功远程兑换血量的次数
    bool free_revive_available;                             // 当前是否可以确认免费复活 0=否 1=是
    bool instant_revive_available;                          // 当前是否可以兑换立即复活 0=否 1=是
    uint16_t instant_revive_cost;                           // 兑换立即复活需要的金币数
    bool out_of_combat;                                     // 当前是否处于脱战状态 0=否 1=是
    uint16_t team_projectile_allowance_17mm_left;           // 队伍 17mm 允许发弹量的剩余可兑换数
    uint8_t posture;                                        // 当前姿态 1=进攻 2=防御 3=移动
    uint8_t energy_mechanism_activate_allowance;            // 己方能量机关是否能够进入正在激活状态 1=可激活
    uint8_t enhanced_posture;                               // 当前姿态是否为强化姿态
    uint8_t attack_before_weak_time_left;                   // 进攻姿态弱化前剩余可持续时长 单位：秒
    uint8_t defense_before_weak_time_left;                  // 防御姿态弱化前剩余可持续时长 单位：秒
    uint8_t move_before_weak_time_left;                     // 移动姿态弱化前剩余可持续时长 单位：秒
    uint8_t enhanced_attack_time_left;                      // 强化进攻姿态剩余可持续时长 单位：秒
    uint8_t enhanced_defense_time_left;                     // 强化防御姿态剩余可持续时长 单位：秒
    uint8_t enhanced_move_time_left;                        // 强化移动姿态剩余可持续时长 单位：秒

    // 0x0307 选手端小地图接收路径数据
    uint8_t intention;                                      // 1=到目标点攻击 2=到目标点防守 3=移动到目标点
    uint16_t start_position_x;                              // 路径起点 x 轴坐标
    uint16_t start_position_y;                              // 路径起点 y 轴坐标
    int8_t delta_x[49];                                     // 路径点 x 轴增量数组
    int8_t delta_y[49];                                     // 路径点 y 轴增量数组
    uint16_t sender_id;                                     // 发送者 ID
};

// 自定义控制器信息
struct custom_controller_info
{
    // 0x0302 自定义控制器 -> 机器人（图传）
    uint8_t data[30];    // 自定义数据

    // 0x0309 机器人 -> 自定义控制器（图传）
    uint8_t data[30];    // 自定义数据
};

// 自定义客户端信息
struct custom_client_info
{
    // 0x0310 机器人 -> 自定义客户端（图传）
    uint8_t data[300];    // 自定义数据

    // 0x0311 自定义客户端 -> 机器人（图传）
    uint8_t data[30];    // 自定义数据
};

// 选手端信息
struct player_client_info
{
    // 0x0303 选手端 -> 机器人（图传）
    float target_position_x;    // 目标位置 x 轴坐标
    float target_position_y;    // 目标位置 y 轴坐标
    uint8_t cmd_keyboard;       // 云台手按下的键盘按键通用键值
    uint8_t target_robot_id;    // 对方机器人 ID
    uint16_t cmd_source;        // 信息来源 ID

    // 0x0308 机器人 -> 选手端（图传）
    uint16_t sender_id;         // 发送者的 ID
    uint16_t receiver_id;       // 接收者的 ID
    uint8_t user_data[30];      /* 字符
                                   以 utf-16 格式编码发送，支持显示中文
                                   编码发送时请注意数据的大小端问题 */
};

#pragma endregion
    
}