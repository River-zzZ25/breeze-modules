/**
 * @file feed.cpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief 拨盘状态机、单圈弹位选择、堵转清理与串级 PID 控制实现
 * @version 0.1
 * @date 2026-11-25
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */
#include "feed.hpp"

#include <cassert>
#include <cmath>

namespace breeze::shooter
{
namespace
{

float WrapAngle(float angle)
{
	const float wrapped = std::fmod(angle, 360.0f);
	return wrapped < 0.0f ? wrapped + 360.0f : wrapped;
}
} // namespace

Feed::Feed(const FeedConfig& config)
    : _config(config),
      _positionPid(MakePid(config.PositionPid)),
      _positionSpeedPid(MakePid(config.PositionSpeedPid)),
      _speedPid(MakePid(config.SpeedPid))
{
	assert(_config.Direction != MotorDirection::None);
	assert(_config.Stuck.ConfirmMs > 0);
#ifndef NDEBUG
	if (const auto* relative = etl::get_if<RelativeFeedPosition>(&_config.Position))
	{
		assert(relative->Step > 0.0f && relative->Step <= 360.0f / 2.0f);
		assert(relative->Reset.ResetSpeed != 0.0f);
		assert(relative->Reset.ResetTimeOut > 0);
	}
	else
	{
		const auto& absolute = etl::get<AbsoluteFeedPosition>(_config.Position);
		assert(!absolute.Positions.empty());
		for (std::size_t i = 0; i < absolute.Positions.size(); ++i)
		{
			assert(absolute.Positions[i] >= 0.0f && absolute.Positions[i] < 360.0f);
		}
	}
#endif
	assert(_config.Shoot.ShootTimeOut > 0);
	assert(_config.Shoot.PositionTolerance >= 0);
	assert(_config.Shoot.HandleStuckReverseTimeout > 0 && _config.Shoot.HandleStuckReloadTimeout > 0);
}

void Feed::Update(ShootState shootState, const FeedCmd& command, const FeedFeedback& feedback, float dtMs)
{
	if (shootState == ShootState::Sleep)
	{
		Reset();
		return;
	}

	UpdateStuckTimer(feedback, dtMs);
	UpdateState(command, feedback, dtMs);
	UpdateOutput(command, feedback, dtMs);
}

void Feed::UpdateStuckTimer(const FeedFeedback& feedback, float dtMs)
{
	if (_info.State == FeedState::Sleep || _info.State == FeedState::Stuck || _info.State == FeedState::Reverse)
	{
		_controlInfo.StuckElapsedMs = 0.0f;
		return;
	}

	float torqueDirection = DirectionSign();
	if (_info.State == FeedState::Init && etl::get<RelativeFeedPosition>(_config.Position).Reset.ResetSpeed < 0.0f)
	{
		torqueDirection = -torqueDirection;
	}

	const auto& stuck = _config.Stuck;
	if (std::abs(feedback.Speed) < stuck.SpeedThreshold && feedback.Torque * torqueDirection > stuck.TorqueThreshold)
	{
		_controlInfo.StuckElapsedMs += dtMs;
	}
	else
	{
		_controlInfo.StuckElapsedMs = 0.0f;
	}
}

void Feed::UpdateState(const FeedCmd& command, const FeedFeedback& feedback, float dtMs)
{
	const bool singleRequested =
	    command.Command == FeedCommand::Single && _controlInfo.LastCommand != FeedCommand::Single;
	// 每周期消费边沿, 非待命状态收到的单发不缓存。
	_controlInfo.LastCommand = command.Command;
	const bool isStuck = _controlInfo.StuckElapsedMs >= _config.Stuck.ConfirmMs;
	const auto& shoot = _config.Shoot;
	auto nextState = _info.State;

	switch (_info.State)
	{
	case FeedState::Sleep:
		if (etl::holds_alternative<RelativeFeedPosition>(_config.Position))
		{
			nextState = FeedState::Init;
		}
		else
		{
			_controlInfo.TargetPosition = SelectAbsolutePosition(feedback.Angle, PositionSelection::Nearest);
			nextState = FeedState::Running;
		}
		break;

	case FeedState::Init:
		_controlInfo.RecoveryElapsedMs += dtMs;
		if (isStuck ||
		    _controlInfo.RecoveryElapsedMs >= etl::get<RelativeFeedPosition>(_config.Position).Reset.ResetTimeOut)
		{
			_controlInfo.PositionOrigin = feedback.Angle;
			_controlInfo.TargetPosition = feedback.Angle;
			nextState = FeedState::Running;
		}
		break;

	case FeedState::Running:
		if (isStuck)
		{
			nextState = FeedState::Stuck;
		}
		else if (singleRequested)
		{
			_controlInfo.TargetPosition = StepPosition(_controlInfo.TargetPosition, PositionSelection::Next);
			nextState = FeedState::SingleShoot;
		}
		else if (command.Command == FeedCommand::Repeat)
		{
			nextState = FeedState::RepeatShoot;
		}
		break;

	case FeedState::SingleShoot:
		// 已开始的单发继续完成, 不被 Hold 或 Repeat 中断。
		_controlInfo.ShootElapsedMs += dtMs;
		if (isStuck)
		{
			nextState = FeedState::Stuck;
		}
		else if (IsPositionReached(feedback.Angle))
		{
			nextState = FeedState::Running;
		}
		else if (_controlInfo.ShootElapsedMs >= shoot.ShootTimeOut)
		{
			_controlInfo.TargetPosition = feedback.Angle;
			nextState = FeedState::Running;
		}
		break;

	case FeedState::RepeatShoot:
		if (isStuck)
		{
			nextState = FeedState::Stuck;
		}
		else if (command.Command != FeedCommand::Repeat)
		{
			_controlInfo.TargetPosition = ForwardAlignmentPosition(feedback.Angle);
			nextState = FeedState::Running;
		}
		break;

	case FeedState::Stuck:
		// 从实际卡住的位置退弹, 不沿用未完成的拨弹目标。
		_controlInfo.TargetPosition = StepPosition(feedback.Angle, PositionSelection::Previous);
		nextState = FeedState::Reverse;
		break;

	case FeedState::Reverse:
		_controlInfo.RecoveryElapsedMs += dtMs;
		if (IsPositionReached(feedback.Angle) || _controlInfo.RecoveryElapsedMs >= shoot.HandleStuckReverseTimeout)
		{
			// 重载以退弹目标为基准, 退弹超时也保留这个基准。
			_controlInfo.TargetPosition = StepPosition(_controlInfo.TargetPosition, PositionSelection::Next);
			nextState = FeedState::Reload;
		}
		break;

	case FeedState::Reload:
		_controlInfo.RecoveryElapsedMs += dtMs;
		if (isStuck)
		{
			nextState = FeedState::Stuck;
		}
		else if (IsPositionReached(feedback.Angle))
		{
			nextState = FeedState::Running;
		}
		else if (_controlInfo.RecoveryElapsedMs >= shoot.HandleStuckReloadTimeout)
		{
			_controlInfo.TargetPosition = feedback.Angle;
			nextState = FeedState::Running;
		}
		break;
	}

	// 仅在状态变化时清除控制历史, 保留本周期选定的目标位置。
	if (nextState == _info.State)
	{
		return;
	}

	_positionPid.Reset();
	_positionSpeedPid.Reset();
	_speedPid.Reset();
	_controlInfo.StuckElapsedMs = 0.0f;
	_controlInfo.RecoveryElapsedMs = 0.0f;
	_controlInfo.ShootElapsedMs = 0.0f;
	_info.State = nextState;
}

void Feed::UpdateOutput(const FeedCmd& command, const FeedFeedback& feedback, float dtMs)
{
	if (_info.State == FeedState::Stuck)
	{
		_output = 0.0f;
		return;
	}

	float targetSpeed{};
	if (_info.State == FeedState::Init)
	{
		targetSpeed = etl::get<RelativeFeedPosition>(_config.Position).Reset.ResetSpeed * DirectionSign();
	}
	else if (_info.State == FeedState::RepeatShoot)
	{
		targetSpeed = command.TargetSpeed * DirectionSign();
	}
	else
	{
		targetSpeed = UpdatePid(_config.PositionPid, CalculatePositionError(feedback.Angle), dtMs, _positionPid);
	}

	const bool speedControl = _info.State == FeedState::Init || _info.State == FeedState::RepeatShoot;
	const auto& speedConfig = speedControl ? _config.SpeedPid : _config.PositionSpeedPid;
	auto& speedPid = speedControl ? _speedPid : _positionSpeedPid;
	_output = UpdatePid(speedConfig, targetSpeed - feedback.Speed, dtMs, speedPid);
}

float Feed::SelectAbsolutePosition(float angle, PositionSelection selection) const
{
	const auto positions = etl::get<AbsoluteFeedPosition>(_config.Position).Positions;
	std::size_t index = 0;
	float error = std::remainder(positions[0] - angle, 360.0f);
	for (std::size_t i = 1; i < positions.size(); ++i)
	{
		const float candidateError = std::remainder(positions[i] - angle, 360.0f);
		if (std::abs(candidateError) <= std::abs(error))
		{
			index = i;
			error = candidateError;
		}
	}

	const float forwardError = error * DirectionSign();
	if (selection == PositionSelection::Next && forwardError <= 0.0f)
	{
		index = (index + 1) % positions.size();
	}
	else if (selection == PositionSelection::Previous && forwardError >= 0.0f)
	{
		index = (index + positions.size() - 1) % positions.size();
	}
	return positions[index];
}

float Feed::StepPosition(float angle, PositionSelection selection) const
{
	if (const auto* relative = etl::get_if<RelativeFeedPosition>(&_config.Position))
	{
		const float step = relative->Step * DirectionSign();
		return WrapAngle(angle + (selection == PositionSelection::Previous ? -step : step));
	}
	return SelectAbsolutePosition(angle, selection);
}

float Feed::ForwardAlignmentPosition(float angle) const
{
	if (const auto* relative = etl::get_if<RelativeFeedPosition>(&_config.Position))
	{
		const int direction = DirectionSign();
		const float offset = WrapAngle((angle - _controlInfo.PositionOrigin) * direction);
		const float index = std::floor(offset / relative->Step) + 1.0f;
		return WrapAngle(_controlInfo.PositionOrigin + index * relative->Step * direction);
	}
	return SelectAbsolutePosition(angle, PositionSelection::Next);
}

float Feed::CalculatePositionError(float angle) const
{
	float error = _controlInfo.TargetPosition - angle;
	if (error > 360.0f / 2.0f)
	{
		error -= 360.0f;
	}
	else if (error < -360.0f / 2.0f)
	{
		error += 360.0f;
	}
	if (std::abs(error) == 360.0f / 2.0f)
	{
		error = (_info.State == FeedState::Reverse ? -1.0f : 1.0f) * DirectionSign() * 360.0f / 2.0f;
	}
	return error;
}

bool Feed::IsPositionReached(float angle) const
{
	return std::abs(CalculatePositionError(angle)) <= _config.Shoot.PositionTolerance;
}

int Feed::DirectionSign() const { return _config.Direction == MotorDirection::Reverse ? -1 : 1; }

void Feed::Reset()
{
	_positionPid.Reset();
	_positionSpeedPid.Reset();
	_speedPid.Reset();
	_controlInfo = {};
	_info.State = FeedState::Sleep;
	_output = 0.0f;
}

} // namespace breeze::shooter
