/*
 * Copyright (c) 2026 RobotPilots-SZU
 * SPDX-License-Identifier: Apache-2.0
 */
#include "breeze/referee/channels.h"
#include "breeze/referee/messages.h"

#include <zephyr/logging/log.h>

using namespace breeze::referee;

LOG_MODULE_REGISTER(referee);

ZBUS_CHAN_DEFINE(referee_game_state_chan,
                game_state_info,
                nullptr,
                nullptr,
                ZBUS_OBSERVERS_EMPTY,
                ZBUS_MSG_INIT());
