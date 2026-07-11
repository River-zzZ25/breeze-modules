/*
 * Copyright (c) 2025 RobotPilots-SZU
 * SPDX-License-Identifier: Apache-2.0
 *
 * Chassis module (skeleton).
 *
 * Only header wiring and an empty init() are provided so the module builds
 * and links. Business logic (device acquisition, topic pub/sub, control loop)
 * will be added later.
 */
#include "chassis.hpp"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include "osal.h" 

LOG_MODULE_REGISTER(chassis, CONFIG_BREEZE_MODULE_CHASSIS_LOG_LEVEL);

namespace chassis {

int init(void)
{
    /* TODO: acquire devices via DEVICE_DT_GET(DT_ALIAS(...)),
     *       subscribe to TOPIC_CHASSIS_CMD, start control loop. */
    return 0;
}

} /* namespace chassis */
