/**
 * @file fric.cpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief 摩擦轮控制实现
 * @version 0.7
 * @date 2026-11-18
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#include "fric.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace breeze::shooter
{
Fric::Fric(const FricConfig& config)
    : _config(config),
      _speedPid(MakePid(config.Pid)),
      _info{},
      _output(0.0f)
{
	assert(_config.Direction != MotorDirection::None);
	assert(
	    _config.Pid.InputDeadband >= 0.0f && _config.Pid.MaxOutput >= 0.0f && _config.Pid.MaxIntegral >= 0.0f &&
	    _config.Pid.InputIntegralSeparation >= 0.0f
	);
}

void Fric::Update(ShootState shootState, const FricCmd& command, const FricFeedback& feedback, float dtMs)
{
	_info.Speed = feedback.Speed;

	if (shootState == ShootState::Sleep)
	{
		Reset();
		return;
	}

	if (command.Mode != _info.Mode)
	{
		_speedPid.Reset();
	}
	_info.Mode = command.Mode;

	ReadyCheck(command);
	UpdateOutput(command, dtMs);
}

void Fric::Reset()
{
	_speedPid.Reset();
	_info.Mode = FricState::Sleep;
	_info.Ready = false;
	_output = 0.0f;
}

void Fric::ReadyCheck(const FricCmd& command)
{
	if (_info.Mode != FricState::Running || command.TargetSpeed <= 0.0f ||
	    command.TargetSpeed < _config.ReadyConfig.MinTarget)
	{
		_info.Ready = false;
		return;
	}

	const float tolerance =
	    std::max(_config.ReadyConfig.MinTolerance, _config.ReadyConfig.SpeedRatio * command.TargetSpeed);
	const float target = command.TargetSpeed * PolarityCheck();
	_info.Ready = std::abs(target - _info.Speed) <= tolerance;
}

void Fric::UpdateOutput(const FricCmd& command, float dtMs)
{
	float target = PolarityCheck() * command.TargetSpeed;
	switch (_info.Mode)
	{
	case FricState::Running:
		break;
	case FricState::Reverse:
		target = -target;
		break;
	case FricState::Stop:
		target = 0.0f;
		break;
	default:
		_output = 0.0f;
		return;
	}

	_output = UpdatePid(_config.Pid, target - _info.Speed, dtMs, _speedPid);
}

float Fric::PolarityCheck() const
{
	return _config.Direction == MotorDirection::Reverse ? -1.0f : 1.0f;
}

} // namespace breeze::shooter
