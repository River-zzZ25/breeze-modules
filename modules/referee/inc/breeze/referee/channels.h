/*
 * Copyright (c) 2026 RobotPilots-SZU
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <zephyr/zbus/zbus.h>

#pragma region Rx Messages
// TODO: 补全所有消息

ZBUS_CHAN_DECLARE(
    referee_game_state_chan
);
#pragma endregion

#pragma region Tx Messages

#pragma endregion

#pragma region Interface

/**
 * @brief 裁判系统输入channel, 图传链路/裁判系统链路 的信息从device发到这里, 则会自动转发成上文的rx messages
 * 
 */
ZBUS_CHAN_DECLARE(
    referee_rx_in_chan
);

#pragma endregion