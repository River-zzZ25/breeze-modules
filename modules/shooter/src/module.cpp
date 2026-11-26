/**
 * @file module.cpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief 发射模块静态 Impl、数据缓存与外部 Tick 控制接线
 * @version 0.1
 * @date 2026-11-17
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#include <breeze/shooter/module.hpp>

#include "shooter.hpp"

#include <atomic>

namespace breeze::shooter
{
namespace
{
static_assert(
    std::atomic<bool>::is_always_lock_free && std::atomic<std::uint16_t>::is_always_lock_free &&
    std::atomic<breeze::models::FiringIntent>::is_always_lock_free && std::atomic<float>::is_always_lock_free
);
} // namespace

template <std::size_t FricCount>
struct Module<FricCount>::Impl
{
	Shooter<FricCount> Controller;
	std::atomic<bool> Enabled{};
	std::atomic<std::uint16_t> RemainingHeat{};
	std::atomic<std::uint16_t> AllowShootCount{};
	std::atomic<breeze::models::FiringIntent> Intent{breeze::models::FiringIntent::Idle};
	std::atomic<float> LatestShootSpeed{};
	explicit Impl(const ShooterConfig<FricCount>& config)
	    : Controller(config)
	{}

	/**
	 * @brief 按外部时间间隔读取反馈、更新控制并推送力矩
	 *
	 */
	void Update(Module& module, float dtMs)
	{
		ShooterInput<FricCount> input{
		    .EnableInfo =
		        {.Intent = Intent.load(std::memory_order_relaxed), .Enable = Enabled.load(std::memory_order_relaxed)},
		    .RemainingHeat = RemainingHeat.load(std::memory_order_relaxed),
		    .AllowShootCount = AllowShootCount.load(std::memory_order_relaxed)
		};
		for (std::size_t i = 0; i < FricCount; ++i)
		{
			module.In.FricFeedback[i](input.FricFeedbacks[i]);
		}
		module.In.FeedFeedback(input.Feed);
		Controller.Update(input, dtMs);
		for (std::size_t i = 0; i < FricCount; ++i)
		{
			module.Out.FricOutput[i](Controller.GetFricOutput(i));
		}
		module.Out.FeedOutput(Controller.GetFeedOutput());
	}
};

template <std::size_t FricCount>
Module<FricCount>::Module(const ShooterConfig<FricCount>& config)
    : In{.Tick = [this](const float& dtMs) { _impl->Update(*this, dtMs); },
	     .Enable = [this](const bool& enabled) { _impl->Enabled.store(enabled, std::memory_order_relaxed); },
	     .HeatValue =
	         [this](const std::uint16_t& value) { _impl->RemainingHeat.store(value, std::memory_order_relaxed); },
	     .AllowShootCount =
	         [this](const std::uint16_t& value) { _impl->AllowShootCount.store(value, std::memory_order_relaxed); },
	     .ShooterCmd = [this](
                           const breeze::models::ShooterCmd& command
                       ) { _impl->Intent.store(command.Intent, std::memory_order_relaxed); },
	     .LatestShootSpeed =
	         [this](const float& speed) { _impl->LatestShootSpeed.store(speed, std::memory_order_relaxed); }}
{
	static Impl impl(config);
	_impl = &impl;
}

template <std::size_t FricCount>
void Module<FricCount>::Start()
{
	_impl->Controller.Start();
}

template class Module<2>;
template class Module<3>;
template class Module<6>;

} // namespace breeze::shooter
