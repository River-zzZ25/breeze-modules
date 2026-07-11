/*
 * Copyright (c) 2025 RobotPilots-SZU
 * SPDX-License-Identifier: Apache-2.0
 *
 * Chassis module (skeleton). Business logic is intentionally not implemented
 * yet; this header only declares the public entry points so the module
 * compiles and links cleanly.
 */
#ifndef BREEZE_MODULE_CHASSIS_HPP
#define BREEZE_MODULE_CHASSIS_HPP

namespace breeze {
namespace chassis {

/* Initialize the chassis module (acquire devices, subscribe topics, ...). */
int init(void);

} /* namespace chassis */
} /* namespace breeze */

#endif /* BREEZE_MODULE_CHASSIS_HPP */
