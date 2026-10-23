#include <zephyr/zbus/zbus.h>
#include <breeze/msgpipe/sub.hpp>

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
static protocol::rx::judge_info_t judge_info;
static struct game_state_info game_state;
static struct robot_state_info robot_state;
static struct chassis_state_info chassis_state;
static struct shooter_state_info shooter_state;
static struct player_client_rx_info player_client;
static struct sentry_only_info sentry;
static struct custom_controller_rx_info custom_controller;
static struct custom_client_rx_info custom_client;

void on_raw_package(raw_package pkg)
{
    switch (pkg.cmd_id)
    {
        // 0x0001 比赛状态
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::game_status):
        {
            memcpy(&judge_info.game_status, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::game_status));

            game_state.game_type = judge_info.game_status.game_type;
            game_state.game_progress = judge_info.game_status.game_progress;
            game_state.stage_remain_time = judge_info.game_status.stage_remain_time;

            msgpipe::publish(game_state, K_NO_WAIT);
            break;
        }

        // 0x0003 机器人血量
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::game_robot_hp):
        {
            memcpy(&judge_info.game_robot_hp, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::game_robot_hp));

            game_state.ally_hero_hp = judge_info.game_robot_hp.ally_1_robot_hp;
            game_state.ally_engineer_hp = judge_info.game_robot_hp.ally_2_robot_hp;
            game_state.ally_infantry_3_hp = judge_info.game_robot_hp.ally_3_robot_hp;
            game_state.ally_infantry_4_hp = judge_info.game_robot_hp.ally_4_robot_hp;
            game_state.ally_sentry_hp = judge_info.game_robot_hp.ally_7_robot_hp;
            game_state.ally_outpost_hp = judge_info.game_robot_hp.ally_outpost_hp;
            game_state.ally_base_hp = judge_info.game_robot_hp.ally_base_hp;
            game_state.enemy_outpost_hp = judge_info.game_robot_hp.enemy_outpost_hp;
            game_state.enemy_base_hp = judge_info.game_robot_hp.enemy_base_hp;

            msgpipe::publish(game_state, K_NO_WAIT);
            break;
        }

        // 0x0101 场地事件
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::event_data):
        {
            memcpy(&judge_info.event_data, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::event_data));

            const uint32_t bits = judge_info.event_data.event_data;
            game_state.ally_supply_occ = (bits >> 0) & 1u;
            game_state.ally_supply_occ_rmul = (bits >> 2) & 1u;
            game_state.ally_small_energy_mechanism_state = (bits >> 3) & 3u;
            game_state.ally_big_energy_mechanism_state = (bits >> 5) & 3u;
            game_state.ally_center_highland_occ = (bits >> 7) & 3u;
            game_state.ally_trapezoid_highland_occ = (bits >> 9) & 1u;
            game_state.enemy_dart_last_hit = (bits >> 20) & 7u;
            game_state.ally_center_buff_occ_rmul = (bits >> 23) & 3u;
            game_state.ally_fort_buff_occ = (bits >> 25) & 3u;
            game_state.ally_outpost_buff_occ = (bits >> 27) & 3u;
            game_state.ally_base_buff_occ = (bits >> 29) & 1u;

            msgpipe::publish(game_state, K_NO_WAIT);
            break;
        }

        // 0x0105 飞镖发射
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::dart_info):
        {
            memcpy(&judge_info.dart_info, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::dart_info));

            game_state.ally_dart_remaining_time = judge_info.dart_info.dart_remaining_time;
            game_state.ally_dart_last_hit_target = (judge_info.dart_info.dart_info >> 0) & 7u;
            game_state.last_target_hit_times = (judge_info.dart_info.dart_info >> 3) & 7u;
            game_state.last_target_hit_type = (judge_info.dart_info.dart_info >> 6) & 7u;

            msgpipe::publish(game_state, K_NO_WAIT);
            break;
        }

        // 0x0201 机器人性能体系数据
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::robot_status):
        {
            memcpy(&judge_info.robot_status, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::robot_status));

            robot_state.robot_id = judge_info.robot_status.robot_id;
            robot_state.robot_level = judge_info.robot_status.robot_level;
            robot_state.current_hp = judge_info.robot_status.current_hp;
            robot_state.maximum_hp = judge_info.robot_status.maximum_hp;

            chassis_state.chassis_power_limit = judge_info.robot_status.chassis_power_limit;

            shooter_state.shooter_barrel_cooling_value = judge_info.robot_status.shooter_barrel_cooling_value;
            shooter_state.shooter_barrel_heat_limit = judge_info.robot_status.shooter_barrel_heat_limit;
            shooter_state.bullet_speed_limit = judge_info.robot_status.bullet_speed_limit;

            msgpipe::publish(robot_state, K_NO_WAIT);
            msgpipe::publish(chassis_state, K_NO_WAIT);
            msgpipe::publish(shooter_state, K_NO_WAIT);
            break;
        }

        // 0x0202 实时底盘缓冲能量和射击热量数据
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::power_heat_data):
        {
            memcpy(&judge_info.power_heat_data, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::power_heat_data));

            chassis_state.buffer_energy = judge_info.power_heat_data.buffer_energy;

            shooter_state.shooter_17mm_barrel_heat = judge_info.power_heat_data.shooter_17mm_barrel_heat;
            shooter_state.shooter_42mm_barrel_heat = judge_info.power_heat_data.shooter_42mm_barrel_heat;

            msgpipe::publish(chassis_state, K_NO_WAIT);
            msgpipe::publish(shooter_state, K_NO_WAIT);
            break;
        }

        // 0x0203 机器人位置数据
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::robot_pos):
        {
            memcpy(&judge_info.robot_pos, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::robot_pos));

            robot_state.x = judge_info.robot_pos.x;
            robot_state.y = judge_info.robot_pos.y;
            robot_state.angle = judge_info.robot_pos.angle;

            msgpipe::publish(robot_state, K_NO_WAIT);
            break;
        }

        // 0x0204 机器人增益和底盘能量数据
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::buff):
        {
            memcpy(&judge_info.buff, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::buff));

            robot_state.recovery_buff = judge_info.buff.recovery_buff;
            robot_state.remaining_energy = judge_info.buff.remaining_energy;

            msgpipe::publish(robot_state, K_NO_WAIT);
            break;
        }

        // 0x0206 伤害状态数据
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::hurt_data):
        {
            memcpy(&judge_info.hurt_data, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::hurt_data));

            robot_state.armor_id = judge_info.hurt_data.armor_id;
            robot_state.hp_deduction_reason = judge_info.hurt_data.hp_deduction_reason;

            msgpipe::publish(robot_state, K_NO_WAIT);
            break;
        }

        // 0x0207 实时射击数据
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::shoot_data):
        {
            memcpy(&judge_info.shoot_data, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::shoot_data));

            shooter_state.launching_frequency = judge_info.shoot_data.launching_frequency;
            shooter_state.initial_speed = judge_info.shoot_data.initial_speed;

            msgpipe::publish(shooter_state, K_NO_WAIT);
            break;
        }

        // 0x0208 允许发弹量与剩余金币
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::projectile_allowance):
        {
            memcpy(&judge_info.projectile_allowance, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::projectile_allowance));

            robot_state.remaining_gold_coin = judge_info.projectile_allowance.remaining_gold_coin;

            shooter_state.projectile_allowance_17mm = judge_info.projectile_allowance.projectile_allowance_17mm;
            shooter_state.projectile_allowance_42mm = judge_info.projectile_allowance.projectile_allowance_42mm;

            msgpipe::publish(robot_state, K_NO_WAIT);
            msgpipe::publish(shooter_state, K_NO_WAIT);
            break;
        }

        // 0x0209 机器人 RFID 模块状态
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::rfid_status):
        {
            memcpy(&judge_info.rfid_status, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::rfid_status));

            const uint32_t rfid = judge_info.rfid_status.rfid_status;
            const uint8_t rfid2 = judge_info.rfid_status.rfid_status_2;
            robot_state.ally_base_buff = (rfid >> 0) & 1u;
            robot_state.ally_central_highland_buff = (rfid >> 1) & 1u;
            robot_state.enemy_central_highland_buff = (rfid >> 2) & 1u;
            robot_state.ally_trapezoid_highland_buff = (rfid >> 3) & 1u;
            robot_state.enemy_trapezoid_highland_buff = (rfid >> 4) & 1u;
            robot_state.ally_fly_slope_before = (rfid >> 5) & 1u;
            robot_state.ally_fly_slope_after = (rfid >> 6) & 1u;
            robot_state.enemy_fly_slope_before = (rfid >> 7) & 1u;
            robot_state.enemy_fly_slope_after = (rfid >> 8) & 1u;
            robot_state.ally_central_highland_below = (rfid >> 9) & 1u;
            robot_state.ally_central_highland_above = (rfid >> 10) & 1u;
            robot_state.enemy_central_highland_below = (rfid >> 11) & 1u;
            robot_state.enemy_central_highland_above = (rfid >> 12) & 1u;
            robot_state.ally_highway_below = (rfid >> 13) & 1u;
            robot_state.ally_highway_above = (rfid >> 14) & 1u;
            robot_state.enemy_highway_below = (rfid >> 15) & 1u;
            robot_state.enemy_highway_above = (rfid >> 16) & 1u;
            robot_state.ally_fort_buff = (rfid >> 17) & 1u;
            robot_state.ally_outpost_buff = (rfid >> 18) & 1u;
            robot_state.ally_supply_no_overlap = (rfid >> 19) & 1u;
            robot_state.ally_supply_overlap = (rfid >> 20) & 1u;
            robot_state.ally_assembly_buff = (rfid >> 21) & 1u;
            robot_state.enemy_assembly_buff = (rfid >> 22) & 1u;
            robot_state.center_buff_rmul = (rfid >> 23) & 1u;
            robot_state.enemy_fort_buff = (rfid >> 24) & 1u;
            robot_state.enemy_outpost_buff = (rfid >> 25) & 1u;
            robot_state.ally_tunnel_highway_below = (rfid >> 26) & 1u;
            robot_state.ally_tunnel_highway_mid = (rfid >> 27) & 1u;
            robot_state.ally_tunnel_highway_above = (rfid >> 28) & 1u;
            robot_state.ally_tunnel_highland_below = (rfid >> 29) & 1u;
            robot_state.ally_tunnel_highland_mid = (rfid >> 30) & 1u;
            robot_state.ally_tunnel_highland_above = (rfid >> 31) & 1u;
            robot_state.enemy_tunnel_highway_below = (rfid2 >> 0) & 1u;
            robot_state.enemy_tunnel_highway_mid = (rfid2 >> 1) & 1u;
            robot_state.enemy_tunnel_highway_above = (rfid2 >> 2) & 1u;
            robot_state.enemy_tunnel_highland_below = (rfid2 >> 3) & 1u;
            robot_state.enemy_tunnel_highland_mid = (rfid2 >> 4) & 1u;
            robot_state.enemy_tunnel_highland_above = (rfid2 >> 5) & 1u;

            msgpipe::publish(robot_state, K_NO_WAIT);
            break;
        }

        // 0x020B 地面机器人位置数据
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::ground_robot_pos):
        {
            memcpy(&judge_info.ground_robot_pos, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::ground_robot_pos));

            sentry.ally_hero_x = judge_info.ground_robot_pos.hero_x;
            sentry.ally_hero_y = judge_info.ground_robot_pos.hero_y;
            sentry.ally_engineer_x = judge_info.ground_robot_pos.engineer_x;
            sentry.ally_engineer_y = judge_info.ground_robot_pos.engineer_y;
            sentry.ally_infantry_3_x = judge_info.ground_robot_pos.standard_3_x;
            sentry.ally_infantry_3_y = judge_info.ground_robot_pos.standard_3_y;
            sentry.ally_infantry_4_x = judge_info.ground_robot_pos.standard_4_x;
            sentry.ally_infantry_4_y = judge_info.ground_robot_pos.standard_4_y;

            msgpipe::publish(sentry, K_NO_WAIT);
            break;
        }

        // 0x020D 哨兵自主决策信息同步
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::sentry_info):
        {
            memcpy(&judge_info.sentry_info, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::sentry_info));

            const uint32_t info = judge_info.sentry_info.sentry_info;
            const uint16_t info2 = judge_info.sentry_info.sentry_info_2;
            const uint64_t info3 = judge_info.sentry_info.sentry_info_3;
            sentry.exchange_projectile_allowance = (info >> 0) & 0x7FFu;
            sentry.remote_exchange_projectile_allowance_times = (info >> 11) & 0xFu;
            sentry.remote_exchange_hp_times = (info >> 15) & 0xFu;
            sentry.free_revive_available = (info >> 19) & 1u;
            sentry.instant_revive_available = (info >> 20) & 1u;
            sentry.instant_revive_cost = (info >> 21) & 0x3FFu;
            sentry.out_of_combat = (info2 >> 0) & 1u;
            sentry.team_projectile_allowance_17mm_left = (info2 >> 1) & 0x7FFu;
            sentry.posture = (info2 >> 12) & 3u;
            sentry.energy_mechanism_activate_allowance = (info2 >> 14) & 1u;
            sentry.enhanced_posture = (info2 >> 15) & 1u;
            sentry.attack_before_weak_time_left = (info3 >> 0) & 0xFFu;
            sentry.defense_before_weak_time_left = (info3 >> 8) & 0xFFu;
            sentry.move_before_weak_time_left = (info3 >> 16) & 0xFFu;
            sentry.enhanced_attack_time_left = (info3 >> 32) & 0xFFu;
            sentry.enhanced_defense_time_left = (info3 >> 40) & 0xFFu;
            sentry.enhanced_move_time_left = (info3 >> 48) & 0xFFu;

            msgpipe::publish(sentry, K_NO_WAIT);
            break;
        }

        // 0x0302 自定义控制器 -> 机器人（图传）
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::custom_controller_data):
        {
            memcpy(&judge_info.custom_controller_data, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::custom_controller_data));

            memcpy(custom_controller.custom_controller_data,
                   judge_info.custom_controller_data.data,
                   sizeof(custom_controller.custom_controller_data));

            msgpipe::publish(custom_controller, K_NO_WAIT);
            break;
        }

        // 0x0303 选手端 -> 机器人（图传）
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::map_command):
        {
            memcpy(&judge_info.map_command, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::map_command));

            player_client.target_position_x = judge_info.map_command.target_position_x;
            player_client.target_position_y = judge_info.map_command.target_position_y;
            player_client.cmd_keyboard = judge_info.map_command.cmd_keyboard;
            player_client.target_robot_id = judge_info.map_command.target_robot_id;
            player_client.cmd_source = judge_info.map_command.cmd_source;

            msgpipe::publish(player_client, K_NO_WAIT);
            break;
        }

        // 0x0307 选手端小地图接收路径数据
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::map_data):
        {
            memcpy(&judge_info.map_data, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::map_data));

            sentry.intention = judge_info.map_data.intention;
            sentry.start_position_x = judge_info.map_data.start_position_x;
            sentry.start_position_y = judge_info.map_data.start_position_y;
            memcpy(sentry.delta_x, judge_info.map_data.delta_x, sizeof(sentry.delta_x));
            memcpy(sentry.delta_y, judge_info.map_data.delta_y, sizeof(sentry.delta_y));
            sentry.sender_id = judge_info.map_data.sender_id;

            msgpipe::publish(sentry, K_NO_WAIT);
            break;
        }

        // 0x0311 自定义客户端 -> 机器人（图传）
        case static_cast<uint16_t>(protocol::rx::RefereeRxID::custom_client_data):
        {
            memcpy(&judge_info.custom_client_data, 
                   pkg.payload.data(), 
                   static_cast<size_t>(protocol::rx::RefereeRxlength::custom_client_data));

            memcpy(custom_client.custom_client_data,
                   judge_info.custom_client_data.data,
                   sizeof(custom_client.custom_client_data));

            msgpipe::publish(custom_client, K_NO_WAIT);
            break;
        }
    }
}

}// namespace internal 
}// namespace breeze::referee
