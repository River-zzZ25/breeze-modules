/*
 * Copyright (c) 2026 RobotPilots-SZU
 * SPDX-License-Identifier: Apache-2.0
 */
// #include "breeze/referee/channels.h"
#include "breeze/referee/messages.h"

#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>

using namespace breeze::referee;

LOG_MODULE_REGISTER(referee);

ZBUS_CHAN_DEFINE(
    referee_game_state_chan,
    game_state_info,
    nullptr,
    nullptr,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT()
);

ZBUS_CHAN_DEFINE(
    referee_robot_state_chan,
    robot_state_info,
    nullptr,
    nullptr,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT()
);

ZBUS_CHAN_DEFINE(
    referee_chassis_state_chan,
    chassis_state_info,
    nullptr,
    nullptr,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT()
);

ZBUS_CHAN_DEFINE(
    referee_shooter_state_chan,
    shooter_state_info,
    nullptr,
    nullptr,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT()
);

ZBUS_CHAN_DEFINE(
    referee_sentry_only_chan,
    sentry_only_info,
    nullptr,
    nullptr,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT()
);

ZBUS_CHAN_DEFINE(
    referee_custom_controller_rx_chan,
    custom_controller_rx_info,
    nullptr,
    nullptr,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT()
);

ZBUS_CHAN_DEFINE(
    referee_custom_client_rx_chan,
    custom_client_rx_info,
    nullptr,
    nullptr,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT()
);

ZBUS_CHAN_DEFINE(
    referee_player_client_rx_chan,
    player_client_rx_info,
    nullptr,
    nullptr,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT()
);

// 还没有rx_in的消息
// ZBUS_CHAN_DEFINE(
//     referee_rx_in_chan,
//     rx_in,
//     nullptr,
//     nullptr,
//     ZBUS_OBSERVERS(referee_rx_distributer),
//     ZBUS_MSG_INIT()
// );
