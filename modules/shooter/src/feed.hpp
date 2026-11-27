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
	FeedCommand Command{FeedCommand::Hold}; ///< 发射指令
	float TargetSpeed{};                    ///< 连发目标角速度(°/s), 拨弹方向为正
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
	 * @param shootState 整机状态; Sleep 时清零输出并重置控制数据
	 * @param command 本周期命令
	 * @param feedback 本周期拨盘轴反馈, 角度为 [0, 360), 保留电机原始极性
	 * @param dtMs 控制周期(ms)
	 * @note 堵转状态由设备层通过 feedback.Stuck 提供并解除, 本类不检测或锁存堵转.
	 *       堵转后自动执行角度环退弹、重载, 完成后使用最新命令.
	 *       退弹到位或超时后重载; 重载到位后保持重载目标, 超时保持当前位置并清零重试计数.
	 *       重载再次堵转时累计次数; 达到 RetryMaxCount 后保持本次堵转位置, 等设备层解除 Stuck 后重新接收命令.
	 *       ShootState::Sleep 立即中止; 相对模式每次唤醒按 Position 中的 Reset 配置复位, 堵转或超时后结束.
	 *       复位后位置环保持并建立弹位基准; 连发结束后对齐拨弹方向的前方第一个弹位.
	 *       绝对模式不机械复位, 唤醒对齐最近弹位; 单发及清堵按表选择前后弹位, 支持跨圈.
	 *       单发到位或 ShootTimeOut 后结束; 超时保持当前位置.
	 *       仅 Running 接收单发边沿, 其它状态不缓存单发指令.
	 */
	void Update(ShootState shootState, const FeedCmd& command, const FeedFeedback& feedback, float dtMs);
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

	/**
	 * @brief 根据本周期角度判断位置目标是否到位, 调用方结合工作状态使用
	 *
	 */
	[[nodiscard]] bool IsPositionReached(float angle) const;

	Feed(const Feed&) = delete;
	Feed(Feed&&) = delete;
	Feed& operator=(const Feed&) = delete;
	Feed& operator=(Feed&&) = delete;

private:
	enum class PositionSelection : std::uint8_t
	{
		Previous,
		Nearest,
		Next
	};

	/**
	 * @brief 检测指令边沿, 按当前状态设置目标并推进状态机
	 *
	 */
	void UpdateState(const FeedCmd& command, const FeedFeedback& feedback, float dtMs);
	/**
	 * @brief 根据当前状态执行位置环、速度环并更新力矩输出
	 *
	 */
	void UpdateOutput(const FeedCmd& command, const FeedFeedback& feedback, float dtMs);
	/**
	 * @brief 按应用层弹位表顺序选择目标, 输入输出均为输出轴原始角度 [0, 360)
	 *
	 * @param angle 角度
	 * @param selection 前一弹位、最近弹位或下一弹位
	 * @return 选定弹位
	 */
	[[nodiscard]] float SelectAbsolutePosition(float angle, PositionSelection selection) const;
	/**
	 * @brief 从给定位置前进或后退一个弹位, selection 为 Next 或 Previous
	 */
	[[nodiscard]] float StepPosition(float angle, PositionSelection selection) const;
	/**
	 * @brief 连发停止后, 从反馈位置向前对齐第一个弹位
	 */
	[[nodiscard]] float ForwardAlignmentPosition(float angle) const;
	/**
	 * @brief 计算目标与当前反馈的有符号圆周误差, 恰好 180 度时按正拨或退弹方向取符号
	 *
	 */
	[[nodiscard]] float CalculatePositionError(float angle) const;
	/**
	 * @brief 拨弹方向到电机方向的换算系数
	 *
	 */
	[[nodiscard]] int DirectionSign() const;

	struct ControlInfo
	{
		FeedCommand LastCommand{FeedCommand::Hold}; ///< 上一周期指令, 用于单发边沿检测
		float RecoveryElapsedMs{};                  ///< 当前复位或清堵阶段已持续时间(ms)
		float PositionOrigin{};                     ///< 复位基准角度 [0, 360), 电机原始极性
		float TargetPosition{};                     ///< 正常拨弹或清堵的当前目标 [0, 360), 电机原始极性
		float ShootElapsedMs{};                     ///< 单发已持续时间(ms)
		std::uint8_t RetryCount{};                  ///< 本轮清堵中重载再次堵转的次数
	};

	FeedConfig _config;       ///< 拨盘配置
	robotpilots::algorithm::Pid<float> _positionPid;
	robotpilots::algorithm::Pid<float> _positionSpeedPid;
	robotpilots::algorithm::Pid<float> _speedPid;
	ControlInfo _controlInfo; ///< 跨周期状态
	FeedInfo _info;           ///< 对外状态
	float _output{};          ///< 本周期电机力矩指令(N·m)
};

} // namespace breeze::shooter
