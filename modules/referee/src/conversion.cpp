#include <zephyr/zbus/zbus.h>
#include <breeze/msgpipe/sub.hpp>

#include "breeze/referee/messages.h"
#include "breeze/referee/channels.h"

using namespace breeze::referee;

ZBUS_CHAN_DEFINE(
    referee_rx_in_chan,
    rx_in,
    nullptr,
    nullptr,
    ZBUS_OBSERVERS(referee_rx_distributer),
    ZBUS_MSG_INIT()
);


class RxHandler
{
public:
    static void on_msg(const rx_in& msg)
    {
        
    }
};

ZBUS_LISTENER_DEFINE(
    referee_rx_distributer,
    (breeze::msgpipe::sub<rx_in,RxHandler>::on_zbus)
);