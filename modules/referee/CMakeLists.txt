# Copyright (c) 2026 RobotPilots-SZU
# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(breeze_module_referee)
file(GLOB REFEREE_SOURCES CONFIGURE_DEPENDS
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "src/*.cpp"
)
zephyr_library_sources(${REFEREE_SOURCES})
target_include_directories(breeze_module_referee
    PUBLIC
        "inc"
)
target_link_libraries(breeze_module_referee
    PUBLIC
        breeze_module_msgpipe
)
