/*
 * Copyright (c) 2026 RobotPilots-SZU
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stats.h"

#include <zephyr/stats/stats.h>
#include <zephyr/sys/util.h>

namespace breeze::referee::internal::stats
{

#ifdef CONFIG_STATS
STATS_SECT_START(referee_stats)
STATS_SECT_ENTRY(RxBytes)
STATS_SECT_ENTRY(RxChunks)
STATS_SECT_ENTRY(RxQueueFull)
STATS_SECT_ENTRY(RxUartStopped)
STATS_SECT_ENTRY(ParserSofFound)
STATS_SECT_ENTRY(ParserHeaderCrcError)
STATS_SECT_ENTRY(ParserLengthError)
STATS_SECT_ENTRY(ParserFrameCrcError)
STATS_SECT_ENTRY(ParserFramesOk)
STATS_SECT_ENTRY(ParserDiscardBytes)
STATS_SECT_ENTRY(RawPackagesEmitted)
STATS_SECT_END;

STATS_NAME_START(referee_stats)
STATS_NAME(referee_stats, RxBytes)
STATS_NAME(referee_stats, RxChunks)
STATS_NAME(referee_stats, RxQueueFull)
STATS_NAME(referee_stats, RxUartStopped)
STATS_NAME(referee_stats, ParserSofFound)
STATS_NAME(referee_stats, ParserHeaderCrcError)
STATS_NAME(referee_stats, ParserLengthError)
STATS_NAME(referee_stats, ParserFrameCrcError)
STATS_NAME(referee_stats, ParserFramesOk)
STATS_NAME(referee_stats, ParserDiscardBytes)
STATS_NAME(referee_stats, RawPackagesEmitted)
STATS_NAME_END(referee_stats);

static STATS_SECT_DECL(referee_stats) referee_stats;

void Init()
{
	const int registrationResult = STATS_INIT_AND_REG(referee_stats, STATS_SIZE_32, "referee");
	ARG_UNUSED(registrationResult);
}

void Record(Event value, uint32_t amount)
{
	switch (value)
	{
	case Event::RxBytes:
		STATS_INCN(referee_stats, RxBytes, amount);
		break;
	case Event::RxChunks:
		STATS_INCN(referee_stats, RxChunks, amount);
		break;
	case Event::RxQueueFull:
		STATS_INCN(referee_stats, RxQueueFull, amount);
		break;
	case Event::RxUartStopped:
		STATS_INCN(referee_stats, RxUartStopped, amount);
		break;
	case Event::ParserSofFound:
		STATS_INCN(referee_stats, ParserSofFound, amount);
		break;
	case Event::ParserHeaderCrcError:
		STATS_INCN(referee_stats, ParserHeaderCrcError, amount);
		break;
	case Event::ParserLengthError:
		STATS_INCN(referee_stats, ParserLengthError, amount);
		break;
	case Event::ParserFrameCrcError:
		STATS_INCN(referee_stats, ParserFrameCrcError, amount);
		break;
	case Event::ParserFramesOk:
		STATS_INCN(referee_stats, ParserFramesOk, amount);
		break;
	case Event::ParserDiscardBytes:
		STATS_INCN(referee_stats, ParserDiscardBytes, amount);
		break;
	case Event::RawPackagesEmitted:
		STATS_INCN(referee_stats, RawPackagesEmitted, amount);
		break;
	}
}
#endif

} // namespace breeze::referee::internal::stats
