#include "pch.h"
#include "socket.h"
#include "mbed-trace/mbed_trace.h"
#include "wifi_helper.h"

namespace GSH {

    bool Socket::init() 
    {
        if (!m_Net) 
        {
            printf("Error! No network interface found.\r\n");
            return false;
        }
        return true;
    }

    bool Socket::connect()
    {
        printf("Connecting to the network...\r\n");
        
        if (!wifi_connect()) 
        {
            // TODO: logger
            return false;
        }

        print_network_info();

        if (!socket_open())
        {
            // TODO: logger
            return false;
        }

        if (!address_initialize())
        {
            // TODO: logger
            return false;
        }
        
        printf("Opening connection to remote port %d\r\n", REMOTE_PORT);

        if(!socket_connect())
        {
            // TODO: logger
            return false;
        };
        return true;
    }

    void Socket::wifi_scan() 
    {
        WiFiInterface *wifi = m_Net->wifiInterface();

        WiFiAccessPoint ap[MAX_NUMBER_OF_ACCESS_POINTS];

        /* scan call returns number of access points found */
        int result = wifi->scan(ap, MAX_NUMBER_OF_ACCESS_POINTS);

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


    void Socket::send(char *json, int len) 
    {
        nsapi_size_or_error_t result;
        result = m_Socket->send(json, len);
        while (0 >= result) 
        {
            GSH_ERROR("Error seding: %d\n", result);
            socket_connect();
            wait_us(5 * SECONDS);
            result = m_Socket->send(json, len);
        }
    }

    bool Socket::recv_http_response()
    {
        char buffer[MAX_MESSAGE_RECEIVED_LENGTH];
        int remaining_bytes = MAX_MESSAGE_RECEIVED_LENGTH;
        int received_bytes = 0;

        /* loop until there is nothing received or we've ran out of buffer space */
        nsapi_size_or_error_t result = remaining_bytes;
        while (result > 0 && remaining_bytes > 0) 
        {
            result = m_Socket->recv(buffer + received_bytes, remaining_bytes);
            if (result < 0) 
            {
                printf("Error! _socket.recv() returned: %d\r\n", result);
                return false;
            }

            received_bytes += result;
            remaining_bytes -= result;
        }

        /* the message is likely larger but we only want the HTTP response code */

        GSH_INFO("received %d bytes:\r\n%.*s\r\n\r\n", received_bytes, strstr(buffer, "\n") - buffer, buffer);

        return true;
    }

    bool Socket::wifi_connect() 
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

    bool Socket::address_initialize() 
    {
        int retry_count = 0;
        while (!resolve_hostname()) 
        {
            if (retry_count > MAX_HOST_RESOLVE_RETRY_COUNT)
            {
                GSH_WARN("address_initialize reach maximum retry count");
                return false;
            }
            wait_us(SECONDS);
        }

        m_Address->set_port(REMOTE_PORT);
        return true;
    }

    bool Socket::resolve_hostname() 
    {
        const char hostname[] = MBED_CONF_APP_HOSTNAME;

        /* get the host address */
        printf("\nResolve hostname %s\r\n", hostname);
        nsapi_size_or_error_t result = m_Net->gethostbyname(hostname, m_Address);
        if (result != 0)
        {
            printf("Error! gethostbyname(%s) returned: %d\r\n", hostname, result);
            return false;
        }

        printf("%s address is %s\r\n", hostname,
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