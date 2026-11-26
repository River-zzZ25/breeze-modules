/**
 * @file shooter.cpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief Shooter 生命周期、热量限制与摩擦轮清堵协调实现
 * @version 0.1
 * @date 2026-11-17
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "shooter.hpp"

#include <algorithm>

namespace breeze::shooter
{
template <std::size_t FricCount>
Shooter<FricCount>::Shooter(const ShooterConfig<FricCount>& conf)
    : Shooter(conf, std::make_index_sequence<FricCount>{})
{}

template <std::size_t FricCount>
template <std::size_t... Indices>
Shooter<FricCount>::Shooter(const ShooterConfig<FricCount>& conf, std::index_sequence<Indices...>)
    : _conf(conf),
      _fric{{Fric(conf.FricConf[Indices])...}},
      _feed(conf.FeedConf)
{}

template <std::size_t FricCount>
void Shooter<FricCount>::Start()
{
	Reset();
}

template <std::size_t FricCount>
void Shooter<FricCount>::Update(const ShooterInput<FricCount>& input, float dtMs)
{
	const auto intent = input.EnableInfo.Intent;
	const bool singleRequested =
	    intent == breeze::models::FiringIntent::Once && _controlInfo.LastIntent != breeze::models::FiringIntent::Once;
	if (!input.EnableInfo.Enable)
	{
		Reset();
		_controlInfo.LastIntent = intent;
		return;
	}
	_controlInfo.LastIntent = intent;
	const auto limit = UpdateHeatLimit(input.RemainingHeat, input.AllowShootCount);
	const bool repeatRequested =
	    intent == breeze::models::FiringIntent::Continuosly && limit.AllowShoot && limit.RepeatSpeed > 0.0f;
	if (_controlInfo.State == ShootState::Sleep)
	{
		_controlInfo.State = ShootState::Init;
	}

	UpdateFric(input.FricFeedbacks, dtMs);
	const bool fricReady =
	    std::all_of(_fric.begin(), _fric.end(), [](const Fric& fric) { return fric.GetStatus().Ready; });
	FeedCmd command{};
	if (_controlInfo.State != ShootState::Init)
	{
		if (limit.AllowShoot && fricReady && singleRequested && _feed.GetStatus().State == FeedState::Running)
		{
			command.Command = FeedCommand::Single;
		}
		else if (fricReady && repeatRequested)
		{
			command.Command = FeedCommand::Repeat;
			command.TargetSpeed = limit.RepeatSpeed;
		}
	}
	_feed.Update(_controlInfo.State, command, input.Feed, dtMs);
	UpdateState(fricReady, input.Feed.Angle);
}

template <std::size_t FricCount>
ShootHeatLimitResult Shooter<FricCount>::UpdateHeatLimit(
    std::uint16_t remainingHeat,
    std::uint16_t allowShootCount
) const
{
	const auto& config = _conf.HeatLimitConf;
	if ((config.AllowShootCountEnable && allowShootCount == 0) ||
	    (config.HeatLimitEnable && remainingHeat <= config.NoShootHeatValue))
	{
		return {};
	}

	float frequency = config.HighHeatShootingFreq;
	if (config.HeatLimitEnable)
	{
		if (remainingHeat <= config.VeryLowHeatValue)
		{
			frequency = config.VeryLowHeatShootingFreq;
		}
		else if (remainingHeat <= config.LowHeatValue)
		{
			frequency = config.LowHeatShootingFreq;
		}
	}
	return {true, frequency * config.FeedSpeedCoefficient};
}

template <std::size_t FricCount>
void Shooter<FricCount>::UpdateState(bool fricReady, float feedAngle)
{
	if (_controlInfo.State == ShootState::Init)
	{
		if (fricReady && _feed.GetStatus().State == FeedState::Running && _feed.IsPositionReached(feedAngle))
		{
			_controlInfo.State = ShootState::Waiting;
		}
		return;
	}

	switch (_feed.GetStatus().State)
	{
	case FeedState::SingleShoot:
		_controlInfo.State = ShootState::SingleShoot;
		break;
	case FeedState::RepeatShoot:
		_controlInfo.State = ShootState::RepeatShoot;
		break;
	default:
		_controlInfo.State = ShootState::Waiting;
		break;
	}
}

template <std::size_t FricCount>
void Shooter<FricCount>::UpdateFric(const etl::array<FricFeedback, FricCount>& feedbacks, float dtMs)
{
	if (_controlInfo.FricReversing)
	{
		_controlInfo.FricReverseElapsedMs += dtMs;
		if (_controlInfo.FricReverseElapsedMs >= 500.0f)
		{
			_controlInfo.FricReversing = false;
			_controlInfo.FricReverseElapsedMs = 0.0f;
		}
	}
	for (std::size_t i = 0; i < FricCount; ++i)
	{
		const FricCmd command{
		    .Mode = _controlInfo.FricReversing ? FricState::Reverse : FricState::Running,
		    .TargetSpeed = _controlInfo.FricReversing ? _conf.FricReverseSpeed
		                                           : _conf.FricConf[i].InintialFricTargetSpeed
		};
		_fric[i].Update(_controlInfo.State, command, feedbacks[i], dtMs);
	}
	if (!_controlInfo.FricReversing && std::any_of(_fric.begin(), _fric.end(), [](const Fric& fric) {
		    return fric.GetStatus().Mode == FricState::Stuck;
	    }))
	{
		_controlInfo.FricReversing = true;
		_controlInfo.FricReverseElapsedMs = 0.0f;
		const FricCmd reverse{.Mode = FricState::Reverse, .TargetSpeed = _conf.FricReverseSpeed};
		for (std::size_t i = 0; i < FricCount; ++i)
		{
			_fric[i].Update(_controlInfo.State, reverse, feedbacks[i], dtMs);
		}
	}
}

template <std::size_t FricCount>
void Shooter<FricCount>::Reset()
{
	_controlInfo = {};
	_feed.Reset();
	for (auto& fric : _fric)
	{
		fric.Reset();
	}
}

template <std::size_t FricCount>
const FricInfo& Shooter<FricCount>::GetFricStatus(std::size_t index) const
{
	return _fric[index].GetStatus();
}

template <std::size_t FricCount>
float Shooter<FricCount>::GetFricOutput(std::size_t index) const
{
	return _fric[index].GetOutput();
}

template <std::size_t FricCount>
const FeedInfo& Shooter<FricCount>::GetFeedStatus() const
{
	return _feed.GetStatus();
}

template <std::size_t FricCount>
float Shooter<FricCount>::GetFeedOutput() const
{
	return _feed.GetOutput();
}

} // namespace breeze::shooter
