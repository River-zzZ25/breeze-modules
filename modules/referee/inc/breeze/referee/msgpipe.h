/**
 * @file msgpipe.h
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 裁判系统模块的msgpipe集成
 * @version 0.1
 * @date 2026-10-17
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once
#include <breeze/msgpipe/pub.hpp>
#include <zephyr/sys/clock.h>
#include <zephyr/zbus/zbus.h>

#include "breeze/referee/channels.h"
#include "breeze/referee/messages.h"

namespace breeze::msgpipe
{

template <>
struct Pub<breeze::referee::GameStateInfo>
{
	using Msg = breeze::referee::GameStateInfo;
	static void Publish(const Msg& msg, k_timeout_t timeout) { zbus_chan_pub(&refereeGameStateChan, &msg, timeout); }
};

template <>
struct Pub<breeze::referee::RobotStateInfo>
{
	using Msg = breeze::referee::RobotStateInfo;
	static void Publish(const Msg& msg, k_timeout_t timeout) { zbus_chan_pub(&refereeRobotStateChan, &msg, timeout); }
};

template <>
struct Pub<breeze::referee::ChassisStateInfo>
{
	using Msg = breeze::referee::ChassisStateInfo;
	static void Publish(const Msg& msg, k_timeout_t timeout) { zbus_chan_pub(&refereeChassisStateChan, &msg, timeout); }
};

template <>
struct Pub<breeze::referee::ShooterStateInfo>
{
	using Msg = breeze::referee::ShooterStateInfo;
	static void Publish(const Msg& msg, k_timeout_t timeout) { zbus_chan_pub(&refereeShooterStateChan, &msg, timeout); }
};

template <>
struct Pub<breeze::referee::SentryOnlyInfo>
{
	using Msg = breeze::referee::SentryOnlyInfo;
	static void Publish(const Msg& msg, k_timeout_t timeout) { zbus_chan_pub(&refereeSentryOnlyChan, &msg, timeout); }
};

template <>
struct Pub<breeze::referee::CustomControllerRxInfo>
{
	using Msg = breeze::referee::CustomControllerRxInfo;
	static void Publish(const Msg& msg, k_timeout_t timeout)
	{
		zbus_chan_pub(&refereeCustomControllerRxChan, &msg, timeout);
	}
};

template <>
struct Pub<breeze::referee::CustomClientRxInfo>
{
	using Msg = breeze::referee::CustomClientRxInfo;
	static void Publish(const Msg& msg, k_timeout_t timeout)
	{
		zbus_chan_pub(&refereeCustomClientRxChan, &msg, timeout);
	}
};

template <>
struct Pub<breeze::referee::PlayerClientRxInfo>
{
	using Msg = breeze::referee::PlayerClientRxInfo;
	static void Publish(const Msg& msg, k_timeout_t timeout)
	{
		zbus_chan_pub(&refereePlayerClientRxChan, &msg, timeout);
	}
};

} // namespace breeze::msgpipe
