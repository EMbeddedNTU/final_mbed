#include "mbed.h"

namespace GSH {

    class Socket {
        static constexpr size_t MAX_NUMBER_OF_ACCESS_POINTS = 10;
        static constexpr size_t MAX_MESSAGE_RECEIVED_LENGTH = 100;

        static constexpr size_t REMOTE_PORT = 5555;
        static constexpr int SECONDS = 1000000;

        static constexpr int MAX_WIFI_RETRY_COUNT = 30;
        static constexpr int MAX_OPEN_RETRY_COUNT = 30;
        static constexpr int MAX_HOST_RESOLVE_RETRY_COUNT = 30;
        static constexpr int MAX_CONNECT_RETRY_COUNT = 30;

    public:
        Socket() : m_Net(NetworkInterface::get_default_instance()) {}

        ~Socket() {
            if (m_Net) 
            {
                m_Net->disconnect();
            }
        }

        bool init();

        void wifi_scan();

        bool connect();

        void send(char *json, int len);

        bool recv_http_response();

    private:
        bool wifi_connect();

        void print_network_info();

        bool socket_open();

        bool address_initialize();
        bool resolve_hostname();

        bool socket_connect();

        void socket_restart();

    private:
        NetworkInterface *m_Net;
        TCPSocket *m_Socket = new TCPSocket();
        SocketAddress *m_Address = new SocketAddress();
    };

}