/*
 * Copyright (c) 2026 RobotPilots-SZU
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <cstdint>
#include <zephyr/zbus/zbus.h>

namespace breeze {
namespace referee {

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

/* Subscribers add their own ZBUS_LISTENER_DEFINE / ZBUS_CHAN_ADD_OBS
 * in their module source files. No runtime registration is required.
 */
ZBUS_CHAN_DECLARE(game_state_chan);

/* Publish a complete snapshot. Ordinary listeners run synchronously in the
 * publisher's context; timeout bounds the channel lock wait, not callback time.
 * Return the zbus publication result (0 on success, negative errno on failure).
 */
int game_state_publish(const game_state_info& msg,
                       k_timeout_t timeout = K_MSEC(100));

int game_state_register();

} /* namespace referee */
} /* namespace breeze */
