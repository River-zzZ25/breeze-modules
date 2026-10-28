/*
 * Copyright (c) 2026 RobotPilots-SZU
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <zephyr/zbus/zbus.h>

#pragma region Rx Messages

ZBUS_CHAN_DECLARE(refereeGameStateChan);

ZBUS_CHAN_DECLARE(refereeRobotStateChan);

ZBUS_CHAN_DECLARE(refereeChassisStateChan);

ZBUS_CHAN_DECLARE(refereeShooterStateChan);

ZBUS_CHAN_DECLARE(refereeSentryOnlyChan);

ZBUS_CHAN_DECLARE(refereeCustomControllerRxChan);

ZBUS_CHAN_DECLARE(refereeCustomClientRxChan);

ZBUS_CHAN_DECLARE(refereePlayerClientRxChan);

#pragma endregion

#pragma region Tx Messages

#pragma endregion
