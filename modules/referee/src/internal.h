#include "protocol/parser.h"

#include <zephyr/device.h>

#pragma once

namespace breeze::referee::internal
{
	/**
	 * @brief 解出裸id包后的分发对接口.
	 * 
	 */
	void on_raw_package(raw_package);
	/**
	 * @brief uart启动
	 * @returns bool 是否成功启动
	 */
	bool start_uart(const device* uart, StreamParser& parser, bool is_vt);
}
