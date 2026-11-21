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
      _controlInfo{},
      _info{},
      _output(0.0f)
{
	assert(IsConfigValid());
}

void Fric::Update(const FricCmd& command, const FricFeedback& feedback, float dtMs)
{
	if (command.Mode != _controlInfo.LastMode)
	{
		_controlInfo = {};
		_controlInfo.LastMode = command.Mode;
	}
	_info.Mode = _controlInfo.Stuck ? FricState::Stuck : command.Mode;

	_info.Speed = feedback.Speed;
	_info.Torque = feedback.Torque;

	if (dtMs <= 0.0f || command.TargetSpeed < 0.0f)
	{
		_controlInfo.SpeedPid = {};
		_controlInfo.StallCycles = 0;
		_info.Ready = false;
		_output = 0.0f;
		return;
	}

	StuckCheck(command);
	ReadyCheck(command);
	UpdateOutput(command, dtMs);
}

void Fric::Reset()
{
	_controlInfo = {};
	_info.Mode = FricState::Sleep;
	_info.Ready = false;
	_output = 0.0f;
}

bool Fric::IsConfigValid() const
{
	const auto& pid = _config.Pid;
	if (PolarityCheck() == 0.0f || pid.InputDeadband < 0.0f || pid.MaxOutput < 0.0f || pid.MaxIntegral < 0.0f ||
	    pid.InputIntegralSeparation < 0.0f)
	{
		return false;
	}

	if (!_config.StuckConfig.has_value())
	{
		return true;
	}

	const auto& stuckConfig = *_config.StuckConfig;
	return stuckConfig.TorqueThreshold >= 0.0f && stuckConfig.SpeedThreshold >= 0.0f && stuckConfig.ConfirmCycles >= 1;
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

	if (_controlInfo.StallCycles < stuckConfig.ConfirmCycles)
	{
		++_controlInfo.StallCycles;
	}

	if (_controlInfo.StallCycles >= stuckConfig.ConfirmCycles)
	{
		_controlInfo.Stuck = true;
		_info.Mode = FricState::Stuck;
		_info.Ready = false;
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
	_info.Ready = std::abs(command.TargetSpeed - std::abs(_info.Speed)) <= tolerance;
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

	_output = UpdatePid(_config.Pid, target - _info.Speed, dtMs, _controlInfo.SpeedPid);
}

float Fric::PolarityCheck() const
{
	switch (_config.Direction)
	{
	case MotorDirection::Forward:
		return 1.0f;
	case MotorDirection::Reverse:
		return -1.0f;
	case MotorDirection::None:
	default:
		return 0.0f;
	}
}

} // namespace breeze::shooter
