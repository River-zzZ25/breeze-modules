/**
 * @file pid.hpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief 发射模块的算法库 PID 配置与输入输出适配
 * @version 0.6
 * @date 2026-11-18
 *
 * @note 积分、微分与历史重置由算法库 Pid 负责.
 *       此处只转换配置、周期单位并处理输入死区与输出限幅.
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <algorithm>
#include <breeze/shooter/config.hpp>
#include <cmath>
#include <limits>
#include <robotpilots/algorithm/pid.hpp>

namespace breeze::shooter
{

inline robotpilots::algorithm::Pid<float> MakePid(const PidConfig& config)
{
	// 算法库始终执行积分限幅, 用无穷大保留 shooter 的零值不限幅约定。
	const float integralLimit = config.MaxIntegral > 0.0f ? config.MaxIntegral : std::numeric_limits<float>::infinity();
	return robotpilots::algorithm::Pid<float>({
	    .Kp = config.Kp,
	    .Ki = config.Ki,
	    .Kd = config.Kd,
	    .IntegralLimit = integralLimit,
	    .IntegralSeparation = config.InputIntegralSeparation,
	});
}

/**
 * @brief 处理输入死区与输出限幅, 采样周期从毫秒转换为算法库使用的秒
 */
inline float UpdatePid(const PidConfig& config, float error, float dtMs, robotpilots::algorithm::Pid<float>& pid)
{
	error = std::abs(error) < config.InputDeadband ? 0.0f : error;
	float output = pid.Update(error, dtMs * 0.001f);
	return config.MaxOutput > 0.0f ? std::clamp(output, -config.MaxOutput, config.MaxOutput) : output;
}

} // namespace breeze::shooter
