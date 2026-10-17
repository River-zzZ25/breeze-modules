/**
 * @file sub.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 消息的强类型订阅封装层, 目前基于zbus
 * @version 0.1
 * @date 2026-10-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */


#include <zephyr/zbus/zbus.h>
namespace breeze::msgpipe
{

namespace internal
{
    template <typename TMsg, typename THandler, typename = void>
    struct has_on_msg : std::false_type {};

    template <typename TMsg, typename THandler>
    struct has_on_msg<TMsg, THandler, std::void_t<decltype(
        THandler::on_msg(std::declval<const TMsg&>())
    )>> : std::true_type {};
}

/**
 * @brief 订阅(listen)的辅助模板
 * 后续注册zbus可以直接传递特化sub的on_zbus.
 * 
 * @tparam TMsg 消息类型
 * @tparam THandler 订阅处理类型, 将订阅逻辑写在函数 on_msg(const TMsg&)内
 */
template <typename TMsg, typename THandler>
struct sub
{
    static void on_zbus(const struct zbus_channel *chan)
    {
        static_assert(
            internal::has_on_msg<TMsg, THandler>::value,
            "msgpipe: THandler must provide: "
            "static void on_msg(const TMsg&)."
        );
        const auto *msg = static_cast<const TMsg*>(zbus_chan_const_msg(chan));
        THandler::on_msg(*msg);
    }
};

}