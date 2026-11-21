/**
 * @file pid.hpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief 模块内部 PID 算法
 * @version 0.6
 * @date 2026-11-18
 *
 * @note 只提供算法, 配置由 Fric/Feed 持有, 计算状态通过 PidCalculate 传入.
 *       输入死区、积分分离、积分限幅、输出限幅都在内部处理.
 *       误差、积分、微分与各项输出更新到调用方状态中.
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <algorithm>
#include <breeze/shooter/config.hpp>
#include <cmath>

namespace breeze::shooter
{

/**
 * @brief PID 计算状态
 *
 */
struct PidCalculate
{
	float Integral{};   // 积分累计(误差·s)
	float Derivative{}; // 微分值(误差/s)

	float Error{};     // 死区处理后的误差
	float LastError{}; // 下次计算使用的历史误差

	float Pout{};       // 比例输出
	float Iout{};       // 积分输出
	float Dout{};       // 微分输出
	float Output{};     // 限幅后的总输出
	bool Initialized{}; // 微分历史是否有效
};

/**
 * @brief 根据误差更新 PID
 *
 * @param config PID 配置, 须为合法配置
 * @param error 误差, 须为有限值
 * @param dtMs 采样周期(ms), 须为正的有限值
 * @param state 调用方持有的 PID 计算状态
 * @return 限幅后的输出, 单位由调用方误差与配置决定, Fric 速度环为 N·m;
 */
inline float UpdatePid(const PidConfig& config, float error, float dtMs, PidCalculate& state)
{
	state.Error = std::abs(error) < config.InputDeadband ? 0.0f : error;
	const float dtSeconds = dtMs * 0.001f;

	const bool separated = config.InputIntegralSeparation > 0.0f && std::abs(state.Error) > config.InputIntegralSeparation;
	if (separated)
	{
		state.Integral = 0.0f;
	}
	else
	{
		state.Integral += state.Error * dtSeconds;
		if (config.MaxIntegral > 0.0f)
		{
			state.Integral = std::clamp(state.Integral, -config.MaxIntegral, config.MaxIntegral);
		}
	}

	state.Derivative = state.Initialized ? (state.Error - state.LastError) / dtSeconds : 0.0f;
	state.LastError = state.Error;
	state.Initialized = true;

	state.Pout = config.Kp * state.Error;
	state.Iout = config.Ki * state.Integral;
	state.Dout = config.Kd * state.Derivative;
	state.Output = state.Pout + state.Iout + state.Dout;
	if (config.MaxOutput > 0.0f)
	{
		state.Output = std::clamp(state.Output, -config.MaxOutput, config.MaxOutput);
	}

	return state.Output;
}

} // namespace breeze::shooter
