#include "protocol/parser.h"

#include <zephyr/device.h>

#pragma once

namespace breeze::referee::internal
{
/**
 * @brief 解出裸id包后的分发对接口.
 *
 */
void OnRawPackage(RawPackage);
/**
 * @brief uart启动
 * @returns bool 是否成功启动
 */
bool StartUart(const device* uart, StreamParser& parser, bool isVt);
} // namespace breeze::referee::internal
