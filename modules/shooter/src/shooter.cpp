/**
 * @file shooter.cpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief
 * @version 0.1
 * @date 2026-11-17
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "shooter.hpp"

namespace breeze::shooter
{
template <std::size_t FricCount>
Shooter<FricCount>::Shooter(const ShooterConfig<FricCount>& conf)
    : Shooter(conf, std::make_index_sequence<FricCount>{})
{}

template <std::size_t FricCount>
template <std::size_t... Indices>
Shooter<FricCount>::Shooter(const ShooterConfig<FricCount>& conf, std::index_sequence<Indices...>)
    : _conf(conf),
      _fric{{Fric(conf.FricConf[Indices])...}},
      _feed(conf.FeedConf)
{}

template <std::size_t FricCount>
void Shooter<FricCount>::Start()
{
	Update(1.0f);
}

template <std::size_t FricCount>
void Shooter<FricCount>::Update(float)
{
	// TODO: 接入周期输入、状态机及 Fric/Feed 控制.
}

template <std::size_t FricCount>
const FricInfo& Shooter<FricCount>::GetFricStatus(std::size_t index) const
{
	return _fric[index].GetStatus();
}

template <std::size_t FricCount>
float Shooter<FricCount>::GetFricOutput(std::size_t index) const
{
	return _fric[index].GetOutput();
}

template <std::size_t FricCount>
const FeedInfo& Shooter<FricCount>::GetFeedStatus() const
{
	return _feed.GetStatus();
}

template <std::size_t FricCount>
float Shooter<FricCount>::GetFeedOutput() const
{
	return _feed.GetOutput();
}

} // namespace breeze::shooter
