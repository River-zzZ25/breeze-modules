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
#include "internal.h"
#include "protocol/parser.h"
#include "stats.h"

#include <etl/array.h>
#include <etl/circular_buffer.h>
#include <etl/span.h>

namespace breeze::referee::internal
{

struct CompoundBuffer
{
	etl::circular_buffer<uint8_t, CONFIG_BREEZE_MODULE_REFEREE_RING_BUF_SIZE> Ring{};
	etl::array<uint8_t, CONFIG_BREEZE_MODULE_REFEREE_LINEAR_BUF_SIZE> Linear{};
} static refereeBuf, vtBuff;

static etl::optional<StreamParser> refereeParser, vtParser;

StreamParser MkParser(CompoundBuffer& buf)
{
	return {buf.Ring, {buf.Linear.begin(), buf.Linear.end()}, etl::make_delegate<internal::OnRawPackage>()};
}
} // namespace breeze::referee::internal

namespace breeze::referee
{

bool Init(etl::optional<const device*> refereeUart, etl::optional<const device*> vtUart)
{
	internal::stats::Init();

	if (refereeUart.has_value())
	{
		internal::refereeParser.emplace(MkParser(internal::refereeBuf));
		auto ok = internal::StartUart(*refereeUart, internal::refereeParser.value(), false);
		if (!ok)
		{
			return false;
		}
	}
	if (vtUart.has_value())
	{
		internal::vtParser.emplace(internal::MkParser(internal::vtBuff));
		auto ok = internal::StartUart(*vtUart, internal::vtParser.value(), true);
		if (!ok)
		{
			return false;
		}
	}
	return true;
}
} // namespace breeze::referee
