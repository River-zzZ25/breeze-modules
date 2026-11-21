/**
 * @file module.cpp
 * @author haito-Xiao (2565975695@qq.com)
 * @brief
 * @version 0.1
 * @date 2026-11-17
 *
 * @copyright Copyright (c)  2026 RobotPilots-SZU.
 * @license SPDX-License-Identifier: Apache-2.0
 *
 */

#include <breeze/shooter/module.hpp>

namespace breeze::shooter
{
template <std::size_t FricCount>
Module<FricCount>::Module(const ShooterConfig<FricCount>& config)
    : _conf(config),
      In{},
      Out{}
{}

} // namespace breeze::shooter
