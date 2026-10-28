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

#include <utility>
#include <zephyr/zbus/zbus.h>
namespace breeze::msgpipe
{

namespace internal
{
template <typename TMsg, typename THandler, typename = void>
struct HasOnMsg : std::false_type
{};

template <typename TMsg, typename THandler>
struct HasOnMsg<TMsg, THandler, std::void_t<decltype(THandler::OnMsg(std::declval<const TMsg&>()))>> : std::true_type
{};
} // namespace internal

/**
 * @brief 订阅(listen)的辅助模板
 * 后续注册zbus可以直接传递特化Sub的OnZbus.
 *
 * @tparam TMsg 消息类型
 * @tparam THandler 订阅处理类型, 将订阅逻辑写在函数 OnMsg(const TMsg&)内
 */
template <typename TMsg, typename THandler>
struct Sub
{
	static void OnZbus(const struct zbus_channel* chan)
	{
		static_assert(
		    internal::HasOnMsg<TMsg, THandler>::value, "msgpipe: THandler must provide: "
		                                               "static void OnMsg(const TMsg&)."
		);
		const auto* msg = static_cast<const TMsg*>(zbus_chan_const_msg(chan));
		THandler::OnMsg(*msg);
	}
};

} // namespace breeze::msgpipe
