# Copyright (c) 2026 RobotPilots-SZU
# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(breeze_module_referee)
zephyr_library_sources(referee.cpp)

# Allow consumers to include <modules/referee/referee.hpp>.
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..)
