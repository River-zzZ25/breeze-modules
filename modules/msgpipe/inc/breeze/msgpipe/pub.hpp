/**
 * @file msgpipe.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 消息的强类型发布封装层, 目前基于zbus
 * @version 0.1
 * @date 2026-10-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once

#include <type_traits>
#include <zephyr/sys/clock.h>

namespace breeze::msgpipe
{

/**
 * @brief 消息发布仿函数约定的主模板 
 * 需要为具体消息类型 `T` 特化本模板, 并实现静态 operator()
 * 
 * @tparam TMsg 消息类型 
 *
 * @par 如何实现（特化约定）
 * 特化 `pub<T>` 并定义以下静态函数：
 * @code{.cpp}
 * template<>
 * struct breeze::msgpipe::pub<MyMessage>
 * {
 *     static void publish(const MyMessage& msg, k_timeout_t timeout)
 *     {
 *         // 实际的 zbus 发布逻辑
 *         zbus_chan_pub(&MYCHANNEL,&msg,timeout);
 *     }
 * };
 * @endcode
 */
template<typename TMsg, typename = void>
struct pub { };

namespace internal
{

    template <typename T, typename = void>
    struct has_publish_impl : std::false_type {};

    template <typename T>
    struct has_publish_impl<
        T,
        std::void_t<
            decltype(pub<T>::publish(
                std::declval<const T&>(),
                std::declval<k_timeout_t>()
            ))
        >
    > : std::true_type {};

}

/**
 * @brief 强类型消息统一发布入口
 * 
 * @tparam TMsg 消息类型, 必须有 pub<TMsg> 特化并实现 publish(const TMsg&, k_timeout_t)
 * @param msg 要发布的消息
 * @param timeout 超时
 */
template<typename TMsg>
void publish(const TMsg& msg, k_timeout_t timeout)
{
    static_assert(
        internal::has_publish_impl<TMsg>::value,
        "msgpipe: pub_traits<YourType> specialization must provide: "
        "static void publish(const YourType&, k_timeout_t).");
    return pub<TMsg>::publish(msg, timeout);
}

}