/**
 * @file module.cpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 裁判系统顶层coordinate实现, 负责创建任务并分发到各实现内.
 * @version 0.1
 * @date 2026-10-18
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "breeze/referee/module.h"
#include "protocol/parser.h"
#include "internal.h"
#include "stats.h"

#include <etl/array.h>
#include <etl/circular_buffer.h>
#include <etl/span.h>

namespace breeze::referee::internal
{

struct CompoundBuffer
{
	etl::circular_buffer<uint8_t, CONFIG_BREEZE_MODULE_REFEREE_RING_BUF_SIZE> ring {};
	etl::array<uint8_t, CONFIG_BREEZE_MODULE_REFEREE_LINEAR_BUF_SIZE> linear {};
} static referee_buf, vt_buff;

static etl::optional<StreamParser> referee_parser, vt_parser;

StreamParser mk_parser(CompoundBuffer& buf)
{
	return {
		buf.ring,
		{ buf.linear.begin(), buf.linear.end() },
		etl::make_delegate<internal::on_raw_package>()
	};
}
}

namespace breeze::referee
{

bool init(etl::optional<const device*> referee_uart, etl::optional<const device*> vt_uart)
{
	internal::stats::init();

	if (referee_uart.has_value())
	{
		internal::referee_parser.emplace(mk_parser(internal::referee_buf));
		auto ok = internal::start_uart(*referee_uart, internal::referee_parser.value(), false);
		if (!ok) return false;
	}
	if (vt_uart.has_value())
	{
		internal::vt_parser.emplace(internal::mk_parser(internal::vt_buff));
		auto ok = internal::start_uart(*vt_uart, internal::vt_parser.value(), true);
		if (!ok) return false;
	}
	return true;
}
}