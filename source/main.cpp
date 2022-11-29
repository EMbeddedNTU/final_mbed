#include "pch.h"
#include "mbed-trace/mbed_trace.h"
#include "wifi_helper.h"
#include "socket.h"


int main() {
    constexpr int SECONDS = 1000000;

#ifdef MBED_CONF_MBED_TRACE_ENABLE
    mbed_trace_init();
#endif

    GSH::Socket *socket = new GSH::Socket();
    MBED_ASSERT(socket);

    socket->init();
    socket->wifi_scan();
    socket->connect();

    char data[3] = {'a', 'b', 'c'};
    while (1) 
    {
        socket->send(data, 3);
        socket->recv_http_response();
        wait_us(SECONDS / 10);
    }

    return 0;
}
