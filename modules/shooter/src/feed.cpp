#include "feed.hpp"

#include <cassert>
#include <cmath>

namespace breeze::shooter
{

Feed::Feed(const FeedConfig& config)
    : _config(config)
{
	assert(_config.Direction != MotorDirection::None);
	assert(_config.Stuck.ConfirmMs > 0);
	assert(!_config.Stuck.TorqueDir.has_value() || *_config.Stuck.TorqueDir == 1 || *_config.Stuck.TorqueDir == -1);
#ifndef NDEBUG
	if (const auto* relative = etl::get_if<RelativeFeedPosition>(&_config.Position))
	{
		assert(relative->Step > 0.0f);
		assert(relative->Reset.ResetSpeed != 0.0f);
		assert(relative->Reset.ResetTimeOut > 0);
	}
	else
	{
		const auto& absolute = etl::get<AbsoluteFeedPosition>(_config.Position);
		assert(absolute.CountsPerTurn > 0.0f && !absolute.Positions.empty());
		for (std::size_t i = 0; i < absolute.Positions.size(); ++i)
		{
			assert(absolute.Positions[i] >= 0.0f && absolute.Positions[i] < absolute.CountsPerTurn);
			assert(i == 0 || absolute.Positions[i - 1] < absolute.Positions[i]);
		}
	}
#endif
	assert(_config.Shoot.ShootTimeOut > 0);
	assert(_config.Shoot.PositionTolerance >= 0.0f);
	assert(_config.Shoot.HandleStuckReverseTimeout > 0 && _config.Shoot.HandleStuckReloadTimeout > 0);
}

void Feed::Update(const FeedCmd& command, const FeedFeedback& feedback, float dtMs)
{
	if (command.Command == FeedCommand::Sleep)
	{
		Reset();
		return;
	}

	if (dtMs <= 0.0f)
	{
		_output = 0.0f;
		return;
	}

	StuckCheck(feedback, dtMs);
	UpdateState(command, feedback, dtMs);
	UpdateOutput(command, feedback, dtMs);
}

float Feed::SelectPosition(float angle, int direction) const
{
	const float polarity = PolarityCheck();
	const auto* relative = etl::get_if<RelativeFeedPosition>(&_config.Position);
	if (relative)
	{
		const float step = relative->Step * polarity;
		return _controlInfo.PositionOrigin +
		       (std::round((angle - _controlInfo.PositionOrigin) / step) + direction) * step;
	}
	const auto& absolute = etl::get<AbsoluteFeedPosition>(_config.Position);
	const float forwardAngle = angle * polarity;
	float nearest = 0.0f;
	float distance = absolute.CountsPerTurn;
	for (const float slot : absolute.Positions)
	{
		const float turns = (forwardAngle - slot) / absolute.CountsPerTurn;
		const float turn = direction > 0   ? std::floor(turns) + 1.0f
		                   : direction < 0 ? std::ceil(turns) - 1.0f
		                                   : std::round(turns);
		const float candidate = slot + turn * absolute.CountsPerTurn;
		if (const float delta = std::abs(candidate - forwardAngle); delta <= distance)
		{
			distance = delta;
			nearest = candidate;
		}
	}
	return nearest * polarity;
}

void Feed::StuckCheck(const FeedFeedback& feedback, float dtMs)
{
	const float polarity = PolarityCheck();
	const auto* relative = etl::get_if<RelativeFeedPosition>(&_config.Position);
	const auto& stuck = _config.Stuck;
	const float torqueDirection = _info.State == FeedState::Init
	                                  ? (relative->Reset.ResetSpeed < 0.0f ? -polarity : polarity)
	                                  : (stuck.TorqueDir.has_value() ? static_cast<float>(*stuck.TorqueDir) : polarity);
	const float torque = feedback.Torque * torqueDirection;
	if ((_info.State == FeedState::Init || _info.State == FeedState::Running || _info.State == FeedState::SingleShoot ||
	     _info.State == FeedState::RepeatShoot || _info.State == FeedState::Reload) &&
	    std::abs(feedback.Speed) < stuck.SpeedThreshold && torque > stuck.TorqueThreshold)
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
	_controlInfo.LastCommand = command.Command;
	const bool isStuck = _controlInfo.StuckElapsedMs >= _config.Stuck.ConfirmMs;
	auto nextState = _info.State;
	switch (_info.State)
	{
	case FeedState::Sleep:
		nextState = etl::holds_alternative<RelativeFeedPosition>(_config.Position) ? FeedState::Init : FeedState::Running;
		break;
	case FeedState::Init:
		_controlInfo.RecoveryElapsedMs += dtMs;
		if (isStuck || _controlInfo.RecoveryElapsedMs >= etl::get<RelativeFeedPosition>(_config.Position).Reset.ResetTimeOut)
		{
			nextState = FeedState::Running;
		}
		break;
	case FeedState::Running:
	case FeedState::RepeatShoot:
		nextState = FeedRunning(command, feedback, singleRequested, isStuck);
		break;
	case FeedState::SingleShoot:
		nextState = FeedSingleShoot(feedback, dtMs, isStuck);
		break;
	case FeedState::Stuck:
	case FeedState::Reverse:
	case FeedState::Reload:
		nextState = FeedRecovery(feedback, dtMs, isStuck);
		break;
	}
	FeedChangeState(nextState, feedback);
}

FeedState Feed::FeedRunning(const FeedCmd& command, const FeedFeedback& feedback, bool singleRequested, bool isStuck)
{
	auto nextState = _info.State;
	if (isStuck)
	{
		nextState = FeedState::Stuck;
	}
	else
	{
		if (_info.State == FeedState::RepeatShoot && command.Command != FeedCommand::Repeat)
		{
			_controlInfo.TargetPosition = SelectPosition(feedback.Angle, 0);
		}
		if (_info.State == FeedState::Running && singleRequested)
		{
			_controlInfo.TargetPosition = SelectPosition(_controlInfo.TargetPosition, 1);
			nextState = FeedState::SingleShoot;
		}
		else
		{
			nextState = command.Command == FeedCommand::Repeat ? FeedState::RepeatShoot : FeedState::Running;
		}
	}
	return nextState;
}

FeedState Feed::FeedSingleShoot(const FeedFeedback& feedback, float dtMs, bool isStuck)
{
	const auto& shoot = _config.Shoot;
	auto nextState = _info.State;
	_controlInfo.ShootElapsedMs += dtMs;
	if (isStuck)
	{
		nextState = FeedState::Stuck;
	}
	else if (std::abs(_controlInfo.TargetPosition - feedback.Angle) <= shoot.PositionTolerance)
	{
		nextState = FeedState::Running;
	}
	else if (_controlInfo.ShootElapsedMs >= shoot.ShootTimeOut)
	{
		_controlInfo.TargetPosition = feedback.Angle;
		nextState = FeedState::Running;
	}
	return nextState;
}

FeedState Feed::FeedRecovery(const FeedFeedback& feedback, float dtMs, bool isStuck)
{
	const float polarity = PolarityCheck();
	const auto* relative = etl::get_if<RelativeFeedPosition>(&_config.Position);
	const auto& shoot = _config.Shoot;
	auto nextState = _info.State;
	auto& recoveryTarget = _controlInfo.RecoveryTargetPosition;
	switch (_info.State)
	{
	case FeedState::Stuck:
		nextState = FeedState::Reverse;
		recoveryTarget = relative ? feedback.Angle - relative->Step * polarity : SelectPosition(feedback.Angle, -1);
		break;
	case FeedState::Reverse:
		_controlInfo.RecoveryElapsedMs += dtMs;
		if (std::abs(recoveryTarget - feedback.Angle) <= shoot.PositionTolerance ||
		    _controlInfo.RecoveryElapsedMs >= shoot.HandleStuckReverseTimeout)
		{
			nextState = FeedState::Reload;
			recoveryTarget = relative ? recoveryTarget + relative->Step * polarity : SelectPosition(recoveryTarget, 1);
		}
		break;
	case FeedState::Reload:
		_controlInfo.RecoveryElapsedMs += dtMs;
		if (isStuck)
		{
			nextState = FeedState::Stuck;
		}
		else if (
		    std::abs(recoveryTarget - feedback.Angle) <= shoot.PositionTolerance ||
		    _controlInfo.RecoveryElapsedMs >= shoot.HandleStuckReloadTimeout
		)
		{
			nextState = FeedState::Running;
		}
		break;
	default:
		break;
	}

	return nextState;
}

void Feed::FeedChangeState(FeedState nextState, const FeedFeedback& feedback)
{
	if (nextState != _info.State)
	{
		if (nextState == FeedState::Running)
		{
			if (_info.State == FeedState::Init)
			{
				_controlInfo.PositionOrigin = feedback.Angle;
				_controlInfo.TargetPosition = feedback.Angle;
			}
			else if (_info.State == FeedState::Sleep || _info.State == FeedState::Reload)
			{
				_controlInfo.TargetPosition = SelectPosition(feedback.Angle, 0);
			}
		}
		_controlInfo.PositionPid = {};
		_controlInfo.SpeedPid = {};
		_controlInfo.StuckElapsedMs = 0.0f;
		_controlInfo.RecoveryElapsedMs = 0.0f;
		_controlInfo.ShootElapsedMs = 0.0f;
		_info.State = nextState;
	}
}

void Feed::UpdateOutput(const FeedCmd& command, const FeedFeedback& feedback, float dtMs)
{
	const float polarity = PolarityCheck();
	const auto* relative = etl::get_if<RelativeFeedPosition>(&_config.Position);
	if (_info.State == FeedState::Stuck)
	{
		_output = 0.0f;
		return;
	}

	float targetPosition = _controlInfo.TargetPosition;
	if (_info.State == FeedState::Reverse || _info.State == FeedState::Reload)
	{
		targetPosition = _controlInfo.RecoveryTargetPosition;
	}

	float targetSpeed = command.TargetSpeed * polarity;
	if (_info.State == FeedState::Init)
	{
		targetSpeed = relative->Reset.ResetSpeed * polarity;
	}
	else if (_info.State != FeedState::RepeatShoot)
	{
		targetSpeed = UpdatePid(_config.PositionPid, targetPosition - feedback.Angle, dtMs, _controlInfo.PositionPid);
	}

	_output = UpdatePid(_config.SpeedPid, targetSpeed - feedback.Speed, dtMs, _controlInfo.SpeedPid);
}

float Feed::PolarityCheck() const { return _config.Direction == MotorDirection::Reverse ? -1.0f : 1.0f; }

void Feed::Reset()
{
	_controlInfo = {};
	_info.State = FeedState::Sleep;
	_output = 0.0f;
}

} // namespace breeze::shooter
