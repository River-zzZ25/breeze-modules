/**
 * @file msgpipe.h
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 裁判系统模块的msgpipe集成
 * @version 0.1
 * @date 2026-10-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once
#include <zephyr/sys/clock.h>
#include <zephyr/zbus/zbus.h>
#include <breeze/msgpipe/pub.hpp>

#include "breeze/referee/messages.h"
#include "breeze/referee/channels.h"

namespace breeze::msgpipe
{
    
template<>
struct pub<breeze::referee::messages::game_state_info>
{
    using Msg = breeze::referee::messages::game_state_info;
    static void publish(const Msg& msg, k_timeout_t timeout)
    {
        zbus_chan_pub(&referee_game_state_chan, &msg, timeout);
    }
};

}
    