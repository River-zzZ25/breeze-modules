/**
 * @file feed.hpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief 拨盘控制接口
 * @version 0.1
 * @date 2026-11-07
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
 * @brief 拨盘指令枚举
 *
 */
enum class FeedCommand : std::uint8_t
{
	Sleep,  ///< 休眠(泄力)
	Hold,   ///< 位置环待命; 已开始的单发继续完成
	Single, ///< 仅 Running 状态下进入该指令时触发一次位置环拨弹
	Repeat  ///< 速度环连续拨弹
};

/**
 * @brief 拨盘控制数据结构体
 *
 */
struct FeedCmd
{
	FeedCommand Command{FeedCommand::Sleep}; ///< 发射指令
	float TargetSpeed{}; ///< 连发目标速度(rpm), 拨弹方向为正
};

/**
 * @brief 拨盘对外状态
 *
 */
struct FeedInfo
{
	FeedState State{FeedState::Sleep}; ///< 当前状态, 清堵期间依次为 Stuck、Reverse、Reload
};

/**
 * @brief 拨盘类
 *
 */
class Feed
{
public:
	/**
	 * @brief 构造函数
	 *
	 * @param config 拨盘配置; 绝对弹位表须在 Feed 生命周期内有效且不修改
	 */
	explicit Feed(const FeedConfig& config);
	/**
	 * @brief 更新拨盘控制周期
	 *
	 * @param command 本周期命令
	 * @param feedback 本周期电机反馈，保留电机原始极性
	 * @param dtMs 控制周期(ms)
	 * @note 堵转后自动执行角度环退弹、重载, 完成后使用最新命令.
	 *       退弹到位或超时后重载; 重载到位或超时后恢复, 再次堵转则重新清堵.
	 *       Sleep 立即中止; 相对模式每次唤醒按 Position 中的 Reset 配置复位, 堵转或超时后结束.
	 *       复位后位置环保持并建立弹位基准; 连发结束后对齐最近弹位.
	 *       绝对模式不机械复位, 唤醒对齐最近弹位; 单发及清堵按表选择前后弹位, 支持跨圈.
	 *       单发到位或 ShootTimeOut 后结束; 超时保持当前位置.
	 *       仅 Running 接收单发边沿, 其它状态不缓存单发指令.
	 */
	void Update(const FeedCmd& command, const FeedFeedback& feedback, float dtMs);
	/**
	 * @brief 中止清堵并清除计时、PID 历史与输出
	 *
	 */
	void Reset();

	/**
	 * @brief 读取拨盘状态
	 */
	[[nodiscard]] const FeedInfo& GetStatus() const { return _info; }

	/**
	 * @brief 读取本周期电机力矩指令(N·m)
	 */
	[[nodiscard]] float GetOutput() const { return _output; }

	Feed(const Feed&) = delete;
	Feed(Feed&&) = delete;
	Feed& operator=(const Feed&) = delete;
	Feed& operator=(Feed&&) = delete;

private:
	/**
	 * @brief 选择弹位, 输入输出均为电机累计坐标; direction 为 -1、0、1
	 *
	 */
	[[nodiscard]] float SelectPosition(float angle, int direction) const;
	/**
	 * @brief 更新连续堵转计时
	 *
	 */
	void StuckCheck(const FeedFeedback& feedback, float dtMs);
	/**
	 * @brief 检测指令边沿并调度状态处理
	 *
	 */
	void UpdateState(const FeedCmd& command, const FeedFeedback& feedback, float dtMs);
	/**
	 * @brief 处理待命、连发及单发指令
	 *
	 */
	[[nodiscard]] FeedState FeedRunning(const FeedCmd& command, const FeedFeedback& feedback, bool singleRequested, bool isStuck);
	/**
	 * @brief 处理单发到位、超时及堵转
	 *
	 */
	[[nodiscard]] FeedState FeedSingleShoot(const FeedFeedback& feedback, float dtMs, bool isStuck);
	/**
	 * @brief 处理堵转后的退弹和重载
	 *
	 */
	[[nodiscard]] FeedState FeedRecovery(const FeedFeedback& feedback, float dtMs, bool isStuck);
	/**
	 * @brief 执行状态进入动作并清零 PID 历史与计时
	 *
	 */
	void FeedChangeState(FeedState nextState, const FeedFeedback& feedback);
	/**
	 * @brief 根据当前状态执行位置环、速度环并更新力矩输出
	 *
	 */
	void UpdateOutput(const FeedCmd& command, const FeedFeedback& feedback, float dtMs);
	/**
	 * @brief 拨弹方向到电机方向的换算系数
	 *
	 */
	[[nodiscard]] float PolarityCheck() const;

	struct ControlInfo
	{
		FeedCommand LastCommand{FeedCommand::Sleep}; ///< 上一周期指令, 用于单发边沿检测
		PidCalculate PositionPid{};                 ///< 位置环计算状态
		PidCalculate SpeedPid{};                    ///< 速度环计算状态
		float StuckElapsedMs{};                     ///< 连续堵转时长
		float RecoveryElapsedMs{};                  ///< 当前复位或清堵阶段已持续时间(ms)
		float RecoveryTargetPosition{};             ///< 当前清堵目标(encoder), 电机原始极性
		float PositionOrigin{};                     ///< 复位完成时的弹位基准(encoder), 电机原始极性
		float TargetPosition{};                     ///< 位置环目标(encoder), 电机原始极性
		float ShootElapsedMs{};                     ///< 单发已持续时间(ms)
	};

	FeedConfig _config;       ///< 拨盘配置
	ControlInfo _controlInfo; ///< 跨周期状态
	FeedInfo _info;           ///< 对外状态
	float _output{};          ///< 本周期电机力矩指令(N·m)
};

} // namespace breeze::shooter
