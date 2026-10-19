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
	STATS_SECT_ENTRY(rx_bytes)
	STATS_SECT_ENTRY(rx_chunks)
	STATS_SECT_ENTRY(rx_queue_full)
	STATS_SECT_ENTRY(rx_uart_stopped)
	STATS_SECT_ENTRY(parser_sof_found)
	STATS_SECT_ENTRY(parser_header_crc_error)
	STATS_SECT_ENTRY(parser_length_error)
	STATS_SECT_ENTRY(parser_frame_crc_error)
	STATS_SECT_ENTRY(parser_frames_ok)
	STATS_SECT_ENTRY(parser_discard_bytes)
	STATS_SECT_ENTRY(raw_packages_emitted)
STATS_SECT_END;

STATS_NAME_START(referee_stats)
	STATS_NAME(referee_stats, rx_bytes)
	STATS_NAME(referee_stats, rx_chunks)
	STATS_NAME(referee_stats, rx_queue_full)
	STATS_NAME(referee_stats, rx_uart_stopped)
	STATS_NAME(referee_stats, parser_sof_found)
	STATS_NAME(referee_stats, parser_header_crc_error)
	STATS_NAME(referee_stats, parser_length_error)
	STATS_NAME(referee_stats, parser_frame_crc_error)
	STATS_NAME(referee_stats, parser_frames_ok)
	STATS_NAME(referee_stats, parser_discard_bytes)
	STATS_NAME(referee_stats, raw_packages_emitted)
STATS_NAME_END(referee_stats);

static STATS_SECT_DECL(referee_stats) referee_stats;

void init()
{
	const int registration_result = STATS_INIT_AND_REG(
		referee_stats,
		STATS_SIZE_32,
		"referee"
	);
	ARG_UNUSED(registration_result);
}

void record(event value, uint32_t amount)
{
	switch (value)
	{
	case event::rx_bytes:
		STATS_INCN(referee_stats, rx_bytes, amount);
		break;
	case event::rx_chunks:
		STATS_INCN(referee_stats, rx_chunks, amount);
		break;
	case event::rx_queue_full:
		STATS_INCN(referee_stats, rx_queue_full, amount);
		break;
	case event::rx_uart_stopped:
		STATS_INCN(referee_stats, rx_uart_stopped, amount);
		break;
	case event::parser_sof_found:
		STATS_INCN(referee_stats, parser_sof_found, amount);
		break;
	case event::parser_header_crc_error:
		STATS_INCN(referee_stats, parser_header_crc_error, amount);
		break;
	case event::parser_length_error:
		STATS_INCN(referee_stats, parser_length_error, amount);
		break;
	case event::parser_frame_crc_error:
		STATS_INCN(referee_stats, parser_frame_crc_error, amount);
		break;
	case event::parser_frames_ok:
		STATS_INCN(referee_stats, parser_frames_ok, amount);
		break;
	case event::parser_discard_bytes:
		STATS_INCN(referee_stats, parser_discard_bytes, amount);
		break;
	case event::raw_packages_emitted:
		STATS_INCN(referee_stats, raw_packages_emitted, amount);
		break;
	}
}
#endif

}
