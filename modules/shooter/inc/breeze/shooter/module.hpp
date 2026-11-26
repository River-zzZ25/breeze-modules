/**
 * @file module.hpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief   发射模块接口
 * @version 0.1
 * @date 2026-11-08
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "breeze/models/control.hpp"
#include "config.hpp"
#include "io.hpp"
#include <cstddef>
#include <cstdint>
#include <etl/array.h>
#include <ramen.hpp>

namespace breeze::shooter
{

/**
 * @brief 常驻的发射模块, 通过外部 Tick 输入驱动 Shooter, 不持有线程
 * @tparam FricCount 摩擦轮数量, 最多 6 路, 当前实例化 2、3、6 路
 * @note 应用中只构造一个实例, 连接端口后调用一次 Start; 输入使用各自的最新值.
 *       Start 后由单一周期源串行推送 Tick, 控制在推送者上下文执行.
 */
template <std::size_t FricCount>
class Module
{
	static_assert(FricCount >= 1 && FricCount <= 6);

public:
	/**
	 * @brief 发射模块输入
	 * @note 用于存储发射模块的输入数据。
	 */
	struct Input
	{
		ramen::Pushable<float> Tick;                          ///< 每次推送执行一个控制周期, 输入为实际间隔(ms)
		ramen::Pushable<bool> Enable;                         ///< false 时在下一控制周期进入 Sleep 并卸力
		ramen::Pushable<std::uint16_t> HeatValue;               ///< 热量值输入(计算出剩余热量后传入)
		ramen::Pushable<std::uint16_t> AllowShootCount;         ///< 允许发弹量
		ramen::Pushable<breeze::models::ShooterCmd> ShooterCmd; ///< 射击命令输入
		ramen::Pushable<float> LatestShootSpeed;                ///< 最新弹速(m/s), 当前仅缓存, 自适应待接入
		etl::array<ramen::Puller<breeze::shooter::FricFeedback>, FricCount> FricFeedback{}; ///< 逐轮原始反馈
		ramen::Puller<breeze::shooter::FeedFeedback> FeedFeedback{}; ///< 拨盘原始反馈
	};

	/**
	 * @brief 发射模块输出
	 * @note 用于存储发射模块的输出数据。
	 */
	struct Output
	{
		etl::array<ramen::Pusher<float>, FricCount> FricOutput{}; ///< 逐轮力矩输出(N·m)
		ramen::Pusher<float> FeedOutput{};                       ///< 拨盘力矩输出(N·m)
		ramen::Pusher<breeze::shooter::ShooterReport> ShooterReport{}; ///< 状态报告预留端口, 当前不推送
	};

	Input In;     ///< 输入接口
	Output Out{}; ///< 输出接口

	/**
	 * @brief 构造常驻内部实现并绑定数据端口
	 *
	 * @param config 发射控制配置
	 */
	explicit Module(const ShooterConfig<FricCount>& config);

	/**
	 * @brief 初始化内部控制, 每个对象在外部周期源启动前调用一次
	 *
	 * @note 不创建线程或启动周期源; 发射使能通过 In.Enable 输入.
	 */
	void Start();

	Module(const Module&) = delete;
	Module(Module&&) = delete;
	Module& operator=(const Module&) = delete;
	Module& operator=(Module&&) = delete;

private:
	struct Impl;
	Impl* _impl{}; ///< 指向常驻的静态实现
};

} // namespace breeze::shooter
