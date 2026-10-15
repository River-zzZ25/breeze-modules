/*
 * Copyright (c) 2026 RobotPilots-SZU
 * SPDX-License-Identifier: Apache-2.0
 */
#include "referee.hpp"

namespace breeze {
namespace referee {

static enum referee_info_type
{
    REFEREE_GAME_STATE_INFO = 0,
    REFEREE_ROBOT_STATE_INFO,
    REFEREE_CHASSIS_STATE_INFO,
    REFEREE_SHOOTER_STATE_INFO,
    REFEREE_SENTRY_ONLY_INFO,
    REFEREE_AUTO_CONTROL_INFO,
    REFEREE_AUTO_TCP_INFO,
    REFEREE_PLAYER_INFO,
    REFEREE_INFO_NUM,
};
static bool referee_info_registered[REFEREE_INFO_NUM] = {false};

/* The referee module owns the channel; consumers own their observations. */
ZBUS_CHAN_DEFINE(game_state_chan,
                 game_state_info,
                 nullptr,
                 nullptr,
                 ZBUS_OBSERVERS_EMPTY,
                 ZBUS_MSG_INIT());

int game_state_publish(const game_state_info& msg, k_timeout_t timeout)
{
    return zbus_chan_pub(&game_state_chan, &msg, timeout);
}

int game_state_register()
{
    if (referee_info_registered[REFEREE_GAME_STATE_INFO])
    {
        LOG_ERR("Game state channel already registered.");
        return -1;
    }
    referee_info_registered[REFEREE_GAME_STATE_INFO] = true;
    LOG_DBG("Game state channel registered.");
    return 0;
}

} /* namespace referee */
} /* namespace breeze */
