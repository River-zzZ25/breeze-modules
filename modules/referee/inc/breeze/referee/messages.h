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
struct game_state_info 
{
    // 0x0001 比赛状态
    uint8_t game_type;                      // 比赛类型
    uint8_t game_progress;                  // 比赛阶段
    uint16_t stage_remain_time;             // 当前阶段剩余时间

    // 0x0003 机器人血量
    uint16_t ally_hero_hp;                   // 己方英雄机器人血量
    uint16_t ally_engineer_hp;               // 己方工程机器人血量
    uint16_t ally_infantry_3_hp;             // 己方3号步兵机器人血量
    uint16_t ally_infantry_4_hp;             // 己方4号步兵机器人血量
    uint16_t ally_sentry_hp;                 // 己方哨兵机器人血量
    uint16_t ally_outpost_hp;                // 己方前哨站血量
    uint16_t ally_base_hp;                   // 己方基地血量
    uint16_t enemy_outpost_hp;               // 敌方前哨站血量
    uint16_t enemy_base_hp;                  // 敌方基地血量

    // 0x0101 场地事件
    bool ally_supply_occ;
    bool ally_supply_occ_rmul;
    uint8_t ally_rune_state;
    uint8_t ally_small_rune_state;
    uint8_t ally_big_rune_state;
    bool ally_center_highland_occ;
    uint8_t ally_trap_highland_occ;
    uint8_t enemy_dart_last_target;
    uint8_t ally_center_buff_occ;
    uint8_t fort_buff_occ;
    uint8_t outpost_buff_occ_rmul;
    bool ally_base_buff_occ;

};

struct robot_state_info
{
    
};

struct chassis_state_info
{
    
};

struct shooter_state_info
{
    
};

struct sentry_only_info
{
    
};

struct auto_control_info
{
    
};

struct auto_tcp_info
{

};

struct player_info
{
    
};

#pragma endregion
    
}