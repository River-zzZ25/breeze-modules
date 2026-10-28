/*
 * Copyright (c) 2026 RobotPilots-SZU
 * SPDX-License-Identifier: Apache-2.0
 */
// #include "breeze/referee/channels.h"
#include "breeze/referee/messages.h"

#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

using namespace breeze::referee;

LOG_MODULE_REGISTER(referee);

ZBUS_CHAN_DEFINE(refereeGameStateChan, GameStateInfo, nullptr, nullptr, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT());

ZBUS_CHAN_DEFINE(refereeRobotStateChan, RobotStateInfo, nullptr, nullptr, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT());

ZBUS_CHAN_DEFINE(refereeChassisStateChan, ChassisStateInfo, nullptr, nullptr, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT());

ZBUS_CHAN_DEFINE(refereeShooterStateChan, ShooterStateInfo, nullptr, nullptr, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT());

ZBUS_CHAN_DEFINE(refereeSentryOnlyChan, SentryOnlyInfo, nullptr, nullptr, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT());

ZBUS_CHAN_DEFINE(
    refereeCustomControllerRxChan,
    CustomControllerRxInfo,
    nullptr,
    nullptr,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT()
);

ZBUS_CHAN_DEFINE(
    refereeCustomClientRxChan,
    CustomClientRxInfo,
    nullptr,
    nullptr,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT()
);

ZBUS_CHAN_DEFINE(
    refereePlayerClientRxChan,
    PlayerClientRxInfo,
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
