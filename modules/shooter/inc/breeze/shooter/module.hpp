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
#include <ramen.hpp>

namespace breeze::shooter
{

template <std::size_t FricCount>
class Module
{
public:
	/**
	 * @brief 发射模块输入
	 * @note 用于存储发射模块的输入数据。
	 */
	struct Input
	{
		ramen::Pushable<std::uint16_t> HeatValue;               ///< 热量值输入
		ramen::Pushable<std::uint16_t> AllowShootCount;         ///< 允许发弹量
		ramen::Pushable<breeze::models::ShooterCmd> ShooterCmd; ///< 射击命令输入
		ramen::Pushable<float> LatestShootSpeed;                ///< 最新发射弹速输入
		ramen::Pushable<FricFeedback> FricFeedback;             ///< 摩擦轮反馈输入
		ramen::Pushable<FeedFeedback> FeedFeedback;             ///< 拨盘反馈
	};

	/**
	 * @brief 发射模块输出
	 * @note 用于存储发射模块的输出数据。
	 */
	struct Output
	{
		ramen::Pusher<float> FricOutput;            ///< 摩擦轮输出
		ramen::Pusher<float> FeedOutput;            ///< 拨盘输出
		ramen::Pusher<ShooterReport> ShooterReport; ///< 发射模块状态输出
	};

	Input In;     ///< 输入接口
	Output Out{}; ///< 输出接口

	/**
	 * @brief 构造函数
	 */
	explicit Module(const ShooterConfig<FricCount>& config);
	/**
	 * @brief 启动发射模块
	 *
	 */
	bool Start();
	/**
	 * @brief 重置发射模块
	 *
	 */
	void Reset();
	Module(const Module&) = delete;
	Module(Module&&) = delete;
	Module& operator=(const Module&) = delete;
	Module& operator=(Module&&) = delete;

private:
	ShooterConfig<FricCount> _conf; ///< 发射模块配置
	struct Impl;
	static constexpr std::size_t IMPL_SIZE = 1024;
	static constexpr std::size_t IMPL_ALIGN = alignof(std::max_align_t);

	Impl* GetImpl() { return reinterpret_cast<Impl*>(_implstorage); }
	Impl const* GetImpl() const { return reinterpret_cast<Impl const*>(_implstorage); }
	alignas(IMPL_ALIGN) std::byte _implstorage[IMPL_SIZE];
	bool _implconstructed{false};
};

} // namespace breeze::shooter
