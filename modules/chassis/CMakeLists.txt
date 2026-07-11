# Copyright (c) 2025 RobotPilots-SZU
# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(breeze_module_chassis)

zephyr_library_sources(chassis.cpp)

zephyr_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
