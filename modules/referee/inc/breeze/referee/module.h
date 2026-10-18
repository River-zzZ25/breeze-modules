/**
 * @file module.h
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 裁判系统模块的顶层封装, 实现从 uart --> zbus 消息 的全链路转化
 * @version 0.1
 * @date 2026-10-18
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include <zephyr/device.h>
#include <etl/optional.h>

namespace breeze::referee
{

/**
 * @brief 初始化裁判系统模块
 * 
 * @param referee_uart 裁判系统串口设备, 可空
 * @param vt_uart 图传机串口设备, 可空
 * @returns bool 是否初始化成功
 */
bool init(etl::optional<const device*> referee_uart = {}, etl::optional<const device*> vt_uart = {});

}
