# Copyright (c) 2025 RobotPilots-SZU
# SPDX-License-Identifier: Apache-2.0

zephyr_interface_library_named(breeze_module_models)

target_include_directories(breeze_module_models
	INTERFACE
		"inc"
)

add_library(breeze::module::models ALIAS breeze_module_models)
