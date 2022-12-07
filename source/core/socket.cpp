#include "pch.h"
#include "socket.h"
#include "mbed-trace/mbed_trace.h"
#include "wifi_helper.h"

namespace GSH {

    bool Socket::init() 
    {
        if (!m_Net) 
        {
            GSH_ERROR("Error! No network interface found.\r\n");
            return false;
        }
        return true;
    }

    bool Socket::connect(const char* hostname, const int port)
    {
        nsapi_connection_status_t wifi_status = m_Wifi->get_connection_status();
        if (wifi_status >= NSAPI_STATUS_DISCONNECTED)
        {
            GSH_INFO("wifi status is %d", wifi_status);
            return false;
        }

        print_network_info();

        if (!socket_open())
        {
            // TODO: logger
            return false;
        }

        if (!address_initialize(hostname, port))
        {
            // TODO: logger
            return false;
        }

        if(!socket_connect())
        {
            // TODO: logger
            return false;
        };

        GSH_INFO("Socket connected to %s %d", m_Hostname, m_Port);
        return true;
    }

    void Socket::wifi_scan() 
    {
        m_Wifi = m_Net->wifiInterface();

        WiFiAccessPoint ap[MAX_NUMBER_OF_ACCESS_POINTS];

        /* scan call returns number of access points found */
        int result = m_Wifi->scan(ap, MAX_NUMBER_OF_ACCESS_POINTS);

        if (result <= 0) 
        {
            printf("WiFiInterface::scan() failed with return value: %d\r\n", result);
            return;
        }

        printf("%d networks available:\r\n", result);

        for (int i = 0; i < result; i++) 
        {
            GSH_INFO("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx "
                    "RSSI: %hhd Ch: %hhd\r\n",
                    ap[i].get_ssid(), get_security_string(ap[i].get_security()),
                    ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
                    ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5],
                    ap[i].get_rssi(), ap[i].get_channel());
        }
        printf("\r\n");
    }

    bool Socket::wifi_connect(const char* ssid, const char* password, nsapi_security security)
    {
        int ret = m_Wifi->connect(ssid, password, security);
        if (ret != 0) 
        {
            GSH_ERROR("WiFi connection error");
            return false;
        }
        GSH_INFO("WiFi connect successful");
        return true;
    }

    bool Socket::send(char *buffer, int len) 
    {
        nsapi_size_t bytes_to_send = len;
        nsapi_size_or_error_t bytes_sent = 0;
        while (bytes_to_send) {
            bytes_sent = m_Socket->send(buffer + bytes_sent, bytes_to_send);
            if (bytes_sent < 0) {
                GSH_ERROR("Error! m_Socket.send() returned: %d", bytes_sent);
                return false;
            } else {
                GSH_INFO("sent %d bytes", bytes_sent);
            }

            bytes_to_send -= bytes_sent;
        }
        GSH_TRACE("Complete message sent");
        return true;
    }

    bool Socket::send_http_request(const std::string& url)
    {
        /* loop until whole request sent */
        const char buffer[] = "GET / HTTP/1.1\r\n"
                              "Host: localhost:3000\r\n"
                              "\r\n";

        nsapi_size_t bytes_to_send = strlen(buffer);
        nsapi_size_or_error_t bytes_sent = 0;

        GSH_INFO("\r\nSending message: \r\n%s", buffer);

        while (bytes_to_send) {
            bytes_sent = m_Socket->send(buffer + bytes_sent, bytes_to_send);
            if (bytes_sent < 0) {
                GSH_ERROR("Error! m_Socket.send() returned: %d", bytes_sent);
                return false;
            } else {
                GSH_INFO("sent %d bytes", bytes_sent);
            }

            bytes_to_send -= bytes_sent;
        }

        GSH_INFO("Complete message sent");

        return true;
    }

    int Socket::recv_chunk(char* buffer, uint32_t length)
    {
        int remaining_bytes = length;
        int received_bytes = 0;

        /* loop until there is nothing received or we've ran out of buffer space */
        nsapi_size_or_error_t result = remaining_bytes;
        while (result > 0 && remaining_bytes > 0) 
        {
            result = m_Socket->recv(buffer + received_bytes, remaining_bytes);
            if (result < 0) 
            {
                printf("Error! _socket.recv() returned: %d\r\n", result);
                return -1;
            }

            received_bytes += result;
            remaining_bytes -= result;
        }

        GSH_INFO("received %d bytes:\r\n%.*s\r\n\r\n", received_bytes, buffer + received_bytes - buffer, buffer);

        return received_bytes;
    }

    bool Socket::wifi_connect_default() 
    {
        nsapi_size_or_error_t result;
        result = m_Net->connect();
        int retry_count = 0;
        while (result != 0) 
        {
            if (retry_count > MAX_WIFI_RETRY_COUNT) 
            {
                GSH_WARN("wifi_connect reach maximum retry count");
                return false;
            }
            retry_count++;
            GSH_WARN("wifi_connect() returned: %d\r\n", result);
            wait_us(SECONDS);
            result = m_Net->connect();
        }
        return true;
    }

    void Socket::print_network_info() 
    {
        /* print the network info */
        SocketAddress a;
        m_Net->get_ip_address(&a);
        GSH_INFO("IP address: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
        m_Net->get_netmask(&a);
        GSH_INFO("Netmask: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
        m_Net->get_gateway(&a);
        GSH_INFO("Gateway: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
    }

    bool Socket::socket_open()
    {
        nsapi_size_or_error_t result;
        result = m_Socket->open(m_Net);
        int retry_count = 0;
        while (result != 0)
        {
            if(retry_count > MAX_OPEN_RETRY_COUNT)
            {
                GSH_WARN("socket_open() reach maximum retry count");
                return false;
            }
            retry_count++;
            GSH_WARN("socket_open() returned: %d\r\n", result);
            socket_restart();
            wait_us(5 * SECONDS);
            result = m_Socket->open(m_Net);
        }
        return true;
    }

    bool Socket::address_initialize(const char* hostname, const int port) 
    {
        m_Hostname = hostname;
        m_Port = port;

        int retry_count = 0;
        while (!resolve_hostname(m_Hostname)) 
        {
            if (retry_count > MAX_HOST_RESOLVE_RETRY_COUNT)
            {
                GSH_WARN("address_initialize reach maximum retry count");
                return false;
            }
            wait_us(SECONDS);
        }

        m_Address->set_port(m_Port);
        return true;
    }

    bool Socket::resolve_hostname(const char* hostname) 
    {
        /* get the host address */
        GSH_INFO("Resolve hostname %s", hostname);
        nsapi_size_or_error_t result = m_Net->gethostbyname(hostname, m_Address);
        if (result != 0)
        {
            GSH_ERROR("Error! gethostbyname(%s) returned: %d", hostname, result);
            return false;
        }

        GSH_INFO("%s address is %s\r\n", hostname,
            (m_Address->get_ip_address() ? m_Address->get_ip_address() : "None"));

        return true;
    }

    bool Socket::socket_connect() 
    {
        nsapi_size_or_error_t result;
        result = m_Socket->connect(*m_Address);
        int retry_count = 0;
        while (result != 0) 
        {
            if (retry_count > MAX_CONNECT_RETRY_COUNT)
            {
                // TODO: logger
                return false;
            }
            retry_count++;
            GSH_WARN("socket_connect() returned: %d\r\n", result);
            if(!socket_open()) 
            {
                return false;
            }
            wait_us(5 * SECONDS);
            result = m_Socket->connect(*m_Address);
        }
        return true;
    }

    void Socket::socket_restart() 
    {
        m_Socket->close();
        m_Socket = new TCPSocket();
    }

}