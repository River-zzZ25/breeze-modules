#include "feed.hpp"

#include <cassert>
#include <cmath>

namespace breeze::shooter
{
namespace
{
constexpr float FULL_TURN = 360.0f;

float WrapAngle(float angle)
{
	const float wrapped = std::fmod(angle, FULL_TURN);
	return wrapped < 0.0f ? wrapped + FULL_TURN : wrapped;
}
} // namespace

Feed::Feed(const FeedConfig& config)
    : _config(config)
{
	assert(_config.Direction != MotorDirection::None);
	assert(_config.Stuck.ConfirmMs > 0);
#ifndef NDEBUG
	if (const auto* relative = etl::get_if<RelativeFeedPosition>(&_config.Position))
	{
		assert(relative->Step > 0.0f && relative->Step <= FULL_TURN / 2.0f);
		assert(relative->Reset.ResetSpeed != 0.0f);
		assert(relative->Reset.ResetTimeOut > 0);
	}
	else
	{
		const auto& absolute = etl::get<AbsoluteFeedPosition>(_config.Position);
		assert(!absolute.Positions.empty());
		for (std::size_t i = 0; i < absolute.Positions.size(); ++i)
		{
			assert(absolute.Positions[i] >= 0.0f && absolute.Positions[i] < FULL_TURN);
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

	StuckCheck(feedback, dtMs);
	UpdateState(command, feedback, dtMs);
	UpdateOutput(command, feedback, dtMs);
}

float Feed::SelectAbsolutePosition(float angle, PositionSelection selection) const
{
	const auto positions = etl::get<AbsoluteFeedPosition>(_config.Position).Positions;
	std::size_t index = 0;
	float error = std::remainder(positions[0] - angle, FULL_TURN);
	for (std::size_t i = 1; i < positions.size(); ++i)
	{
		const float candidateError = std::remainder(positions[i] - angle, FULL_TURN);
		if (std::abs(candidateError) <= std::abs(error))
		{
			index = i;
			error = candidateError;
		}
	}

	const float forwardError = error * PolarityCheck();
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

float Feed::CalculatePositionError(float angle) const
{
	float error = _controlInfo.TargetPosition - angle;
	if (error > FULL_TURN / 2.0f)
	{
		error -= FULL_TURN;
	}
	else if (error < -FULL_TURN / 2.0f)
	{
		error += FULL_TURN;
	}
	if (std::abs(error) == FULL_TURN / 2.0f)
	{
		error = (_info.State == FeedState::Reverse ? -1.0f : 1.0f) * PolarityCheck() * FULL_TURN / 2.0f;
	}
	return error;
}

void Feed::StuckCheck(const FeedFeedback& feedback, float dtMs)
{
	const float polarity = PolarityCheck();
	const auto* relative = etl::get_if<RelativeFeedPosition>(&_config.Position);
	const auto& stuck = _config.Stuck;
	const float torqueDirection =
	    _info.State == FeedState::Init ? (relative->Reset.ResetSpeed < 0.0f ? -polarity : polarity) : polarity;
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
	FeedChangeState(nextState);
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
		const auto* relative = etl::get_if<RelativeFeedPosition>(&_config.Position);
		const int polarity = PolarityCheck();
		if (_info.State == FeedState::RepeatShoot && command.Command != FeedCommand::Repeat)
		{
			if (relative)
			{
				const float offset = WrapAngle((feedback.Angle - _controlInfo.PositionOrigin) * polarity);
				const float index = std::floor(offset / relative->Step) + 1.0f;
				_controlInfo.TargetPosition = WrapAngle(_controlInfo.PositionOrigin + index * relative->Step * polarity);
			}
			else
			{
				_controlInfo.TargetPosition = SelectAbsolutePosition(feedback.Angle, PositionSelection::Next);
			}
		}
		if (_info.State == FeedState::Running && singleRequested)
		{
			_controlInfo.TargetPosition = relative ? WrapAngle(_controlInfo.TargetPosition + relative->Step * polarity)
			                                       : SelectAbsolutePosition(_controlInfo.TargetPosition, PositionSelection::Next);
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
	else if (std::abs(CalculatePositionError(feedback.Angle)) <= shoot.PositionTolerance)
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
	const int polarity = PolarityCheck();
	const auto* relative = etl::get_if<RelativeFeedPosition>(&_config.Position);
	const auto& shoot = _config.Shoot;
	auto nextState = _info.State;
	const float recoveryTarget = _controlInfo.TargetPosition;
	switch (_info.State)
	{
	case FeedState::Stuck:
		nextState = FeedState::Reverse;
		_controlInfo.TargetPosition = relative ? WrapAngle(feedback.Angle - relative->Step * polarity)
		                                       : SelectAbsolutePosition(feedback.Angle, PositionSelection::Previous);
		break;
	case FeedState::Reverse:
		_controlInfo.RecoveryElapsedMs += dtMs;
		if (std::abs(CalculatePositionError(feedback.Angle)) <= shoot.PositionTolerance ||
		    _controlInfo.RecoveryElapsedMs >= shoot.HandleStuckReverseTimeout)
		{
			nextState = FeedState::Reload;
			_controlInfo.TargetPosition = relative ? WrapAngle(recoveryTarget + relative->Step * polarity)
			                                       : SelectAbsolutePosition(recoveryTarget, PositionSelection::Next);
		}
		break;
	case FeedState::Reload:
		_controlInfo.RecoveryElapsedMs += dtMs;
		if (isStuck)
		{
			nextState = FeedState::Stuck;
		}
		else if (std::abs(CalculatePositionError(feedback.Angle)) <= shoot.PositionTolerance)
		{
			nextState = FeedState::Running;
		}
		else if (_controlInfo.RecoveryElapsedMs >= shoot.HandleStuckReloadTimeout)
		{
			_controlInfo.TargetPosition = feedback.Angle;
			nextState = FeedState::Running;
		}
		break;
	default:
		break;
	}

	return nextState;
}

void Feed::FeedChangeState(FeedState nextState)
{
	if (nextState == _info.State)
	{
		return;
	}

	_controlInfo.PositionPid = {};
	_controlInfo.SpeedPid = {};
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

	const bool speedControl = _info.State == FeedState::Init || _info.State == FeedState::RepeatShoot;
	float targetSpeed{};
	if (speedControl)
	{
		if (_info.State == FeedState::Init)
		{
			targetSpeed = etl::get<RelativeFeedPosition>(_config.Position).Reset.ResetSpeed * PolarityCheck();
		}
		else
		{
			targetSpeed = command.TargetSpeed * PolarityCheck();
		}
	}
	else
	{
		targetSpeed = UpdatePid(_config.PositionPid, CalculatePositionError(feedback.Angle), dtMs, _controlInfo.PositionPid);
	}

	const auto& speedPid = speedControl ? _config.SpeedPid : _config.PositionSpeedPid;
	_output = UpdatePid(speedPid, targetSpeed - feedback.Speed, dtMs, _controlInfo.SpeedPid);
}

int Feed::PolarityCheck() const { return _config.Direction == MotorDirection::Reverse ? -1 : 1; }

void Feed::Reset()
{
	_controlInfo = {};
	_info.State = FeedState::Sleep;
	_output = 0.0f;
}

} // namespace breeze::shooter
