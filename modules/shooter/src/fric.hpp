/**
 * @file fric.hpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief 摩擦轮控制(模块内部)
 * @version 0.7
 * @date 2026-11-18
 *
 * @note 只负责单个摩擦轮的控制周期:
 *       - 目标速度方向换算、速度 PID、Ready 判断; 反馈保留电机原始极性
 *       - 堵转检测与锁存, 反转与恢复由上层 Shooter 处理
 *       - 非正周期或负目标时清输出、取消 Ready、清 PID 历史与堵转累计
 *       命令与反馈只作为本周期输入, 不保存副本; 只有跨周期数据留在对象中.
 *       速度单位统一 rpm, 力矩单位统一 N·m.
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "pid.hpp"
#include "state.hpp"
#include <breeze/shooter/config.hpp>
#include <breeze/shooter/io.hpp>

namespace breeze::shooter
{

/**
 * @brief 摩擦轮命令(Shooter下发)
 *
 */
struct FricCmd
{
	FricState Mode{FricState::Sleep}; ///< 工作模式
	float TargetSpeed{};              ///< 目标转速大小(rpm), 不接受负值; 零值不判定 Ready
};

/**
 * @brief 摩擦轮状态
 *
 */
struct FricInfo
{
	FricState Mode{FricState::Sleep}; ///< 控制模式
	float Speed{};                    ///< 实际转速(rpm), 电机正转为正
	float Torque{};                   ///< 力矩(N·m), 电机正转为正
	bool Ready{};                     ///< 达到目标转速
};

/**
 * @brief 摩擦轮
 *
 */
class Fric
{
public:
	/**
	 * @brief 构造函数
	 *
	 * @param config 摩擦轮配置
	 */
	explicit Fric(const FricConfig& config);

	/**
	 * @brief 控制周期更新
	 *
	 * @param command 本周期命令
	 * @param feedback 本周期驱动反馈, 极性为电机正转
	 * @param dtMs 采样周期(ms), 须为有限值; 非正时按异常周期处理
	 */
	void Update(const FricCmd& command, const FricFeedback& feedback, float dtMs);

	/**
	 * @brief 清除堵转锁存与 PID 历史
	 *
	 */
	void Reset();

	/**
	 * @brief 读取状态
	 *
	 */
	[[nodiscard]] const FricInfo& GetStatus() const { return _info; };

	/**
	 * @brief 读取本周期力矩指令
	 *
	 * @return 电机正转极性下的力矩(N·m)
	 */
	[[nodiscard]] float GetOutput() const { return _output; };

	Fric(const Fric&) = delete;
	Fric(Fric&&) = delete;
	Fric& operator=(const Fric&) = delete;
	Fric& operator=(Fric&&) = delete;

private:
	/**
	 * @brief 摩擦轮跨周期状态
	 *
	 */
	struct ControlInfo
	{
		FricState LastMode{FricState::Sleep}; ///< 上一周期命令模式
		std::uint16_t StallCycles{};          ///< 连续满足堵转条件的控制周期数
		bool Stuck{};                         ///< 堵转锁存
		PidCalculate SpeedPid{};              ///< 速度环计算状态
	};
	/**
	 * @brief 验证配置是否有效
	 *
	 * @return true
	 * @return false
	 */
	bool IsConfigValid() const;
	/**
	 * @brief 堵转周期计数, 仅 Running 且目标为正时检测
	 *
	 */
	void StuckCheck(const FricCmd& command);
	/**
	 * @brief 按转速幅值判定到速, 仅 Running 且目标为正、不低于 MinTarget
	 *
	 */
	void ReadyCheck(const FricCmd& command);
	/**
	 * @brief 目标转速转换到电机极性后进行速度环计算
	 *
	 */
	void UpdateOutput(const FricCmd& command, float dtMs);
	/**
	 * @brief 出弹方向与电机正转极性的换算系数
	 *
	 */
	float PolarityCheck() const;

	FricConfig _config;       ///< 摩擦轮配置
	ControlInfo _controlInfo; ///< 跨周期状态
	FricInfo _info;           ///< 实际状态
	float _output;            ///< 本周期力矩指令(N·m)
};

} // namespace breeze::shooter
