#pragma once

#include "core/socket.h"
#include "parsed_url.h"

namespace GSH {

    class HttpService 
    {
    public:
        struct HttpResponse
        {
            parsed_url *request_uri;
            char *body;
            char *status_code;
            int status_code_int;
            char *status_text;
            char *request_headers;
            char *response_headers;
        };

    public:
        HttpService();
        ~HttpService() = default;
        HttpService(HttpService &other) = delete;
        void operator=(const HttpService &) = delete;

        static HttpService* GetInstance();
        bool init(const char* ssid, const char* password, nsapi_security security);

        HttpResponse* http_request(char* http_headers, parsed_url* purl);

        HttpResponse* http_get(char *url, char *custom_headers);

    private:
        bool send_request(char* buffer);

    private:
        Socket m_Socket;
    };

}