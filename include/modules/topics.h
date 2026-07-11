/*
 * Copyright (c) 2025 RobotPilots-SZU
 * SPDX-License-Identifier: Apache-2.0
 *
 * Shared zenoh topic names for inter-module communication.
 *
 * Modules publish/subscribe to these topics; payloads are serialized with
 * the breeze-middleware protocol (nanopb). Kept as a single place so module
 * boundaries stay decoupled (no direct calls between modules).
 */
#ifndef BREEZE_MODULES_TOPICS_H
#define BREEZE_MODULES_TOPICS_H

/* Chassis */
#define TOPIC_CHASSIS_CMD     "breeze/chassis/cmd"
#define TOPIC_CHASSIS_STATE   "breeze/chassis/state"

/* Gimbal */
#define TOPIC_GIMBAL_CMD      "breeze/gimbal/cmd"
#define TOPIC_GIMBAL_STATE    "breeze/gimbal/state"

#endif /* BREEZE_MODULES_TOPICS_H */
