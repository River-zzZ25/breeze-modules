/*
 * Copyright (c) 2026 RobotPilots-SZU
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <zephyr/zbus/zbus.h>

#pragma region Rx Messages

ZBUS_CHAN_DECLARE(
    referee_game_state_chan
);

ZBUS_CHAN_DECLARE(
    referee_robot_state_chan
);

ZBUS_CHAN_DECLARE(
    referee_chassis_state_chan
);

ZBUS_CHAN_DECLARE(
    referee_shooter_state_chan
);

ZBUS_CHAN_DECLARE(
    referee_sentry_only_chan
);

ZBUS_CHAN_DECLARE(
    referee_custom_controller_rx_chan
);

ZBUS_CHAN_DECLARE(
    referee_custom_client_rx_chan
);

ZBUS_CHAN_DECLARE(
    referee_player_client_rx_chan
);

#pragma endregion

#pragma region Tx Messages

#pragma endregion