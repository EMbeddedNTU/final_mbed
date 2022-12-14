#pragma once

#include "mbed.h"

namespace GSH {

    class Socket {
        static constexpr size_t MAX_NUMBER_OF_ACCESS_POINTS = 10;

        static constexpr int SECONDS = 1000000;

        static constexpr int MAX_WIFI_RETRY_COUNT = 15;
        static constexpr int MAX_OPEN_RETRY_COUNT = 15;
        static constexpr int MAX_HOST_RESOLVE_RETRY_COUNT = 15;
        static constexpr int MAX_CONNECT_RETRY_COUNT = 15;

    public:
        Socket() : m_Net(NetworkInterface::get_default_instance()) {}

        ~Socket() {
            if (m_Net) 
            {
                m_Net->disconnect();
            }
            // delete m_Address;
            // delete m_Socket;
        }

        bool init();

        void wifi_scan();

        bool wifi_connect(const char* ssid, const char* password, nsapi_security security = NSAPI_SECURITY_WPA_WPA2);

        bool wifi_connect_default();

        bool connect(const char* hostname, const int port);

        bool send(char *buffer, int len);

        int recv_chunk(char* buffer, uint32_t length);

        void close();

    private:
        void print_network_info();

        bool socket_open();

        bool address_initialize(const char* hostname, const int port);
        bool resolve_hostname(const char* hostname);

        bool socket_connect();

        void socket_restart();

    private:
        NetworkInterface *m_Net;
        WiFiInterface *m_Wifi;
        int m_Port;
        const char* m_Hostname;
        TCPSocket *m_Socket = new TCPSocket();
        SocketAddress *m_Address = new SocketAddress();
    };

}