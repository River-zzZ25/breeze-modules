# Copyright (c) 2026 RobotPilots-SZU
# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(breeze_module_referee)
file(GLOB REFEREE_SOURCES CONFIGURE_DEPENDS
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "src/*.cpp"
)
zephyr_library_sources(${REFEREE_SOURCES})
zephyr_library_include_directories("inc")
