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
      _controlInfo{},
      _info{},
      _output(0.0f)
{
	assert(_config.Direction != MotorDirection::None);
	assert(
	    _config.Pid.InputDeadband >= 0.0f && _config.Pid.MaxOutput >= 0.0f && _config.Pid.MaxIntegral >= 0.0f &&
	    _config.Pid.InputIntegralSeparation >= 0.0f
	);
	if (_config.StuckConfig.has_value())
	{
		assert(
		    _config.StuckConfig->TorqueThreshold >= 0.0f && _config.StuckConfig->SpeedThreshold >= 0.0f &&
		    _config.StuckConfig->ConfirmCycles >= 1
		);
	}
}

void Fric::Update(ShootState shootState, const FricCmd& command, const FricFeedback& feedback, float dtMs)
{
	_info.Speed = feedback.Speed;
	_info.Torque = feedback.Torque;

	if (shootState == ShootState::Sleep)
	{
		Reset();
		return;
	}

	if (command.Mode != _controlInfo.LastMode)
	{
		_speedPid.Reset();
		_controlInfo = {};
		_controlInfo.LastMode = command.Mode;
	}
	_info.Mode = _controlInfo.Stuck ? FricState::Stuck : command.Mode;

	StuckCheck(command);
	ReadyCheck(command);
	UpdateOutput(command, dtMs);
}

void Fric::Reset()
{
	_speedPid.Reset();
	_controlInfo = {};
	_info.Mode = FricState::Sleep;
	_info.Ready = false;
	_output = 0.0f;
}

void Fric::StuckCheck(const FricCmd& command)
{
	if (_controlInfo.Stuck || !_config.StuckConfig.has_value() || _info.Mode != FricState::Running ||
	    command.TargetSpeed <= 0.0f)
	{
		if (!_controlInfo.Stuck)
		{
			_controlInfo.StallCycles = 0;
		}
		return;
	}

	const auto& stuckConfig = *_config.StuckConfig;
	const bool overTorque = std::abs(_info.Torque) > stuckConfig.TorqueThreshold;
	const bool underSpeed = std::abs(_info.Speed) < stuckConfig.SpeedThreshold;
	if (!overTorque || !underSpeed)
	{
		_controlInfo.StallCycles = 0;
		return;
	}

	++_controlInfo.StallCycles;

	if (_controlInfo.StallCycles >= stuckConfig.ConfirmCycles)
	{
		_controlInfo.Stuck = true;
		_info.Mode = FricState::Stuck;
	}
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
