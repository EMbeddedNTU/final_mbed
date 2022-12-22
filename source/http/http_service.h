#pragma once

#include "platform/SharedPtr.h"
#include "core/socket.h"
#include "parsed_url.h"
#include <string>

namespace GSH {

    class HttpService 
    {
    public:
        struct HttpResponse
        {
            SharedPtr<ParsedUrl> request_uri;
            // char* body;
            std::string body;
            char* status_code;
            int status_code_int;
            char* status_text;
            char* request_headers;
            char* response_headers;

            HttpResponse()
            {
                body = nullptr;
                request_headers = nullptr;
                response_headers = nullptr;
                status_code = nullptr;
                status_text = nullptr;
                request_uri = SharedPtr<ParsedUrl>(nullptr);
            }

            ~HttpResponse()
            {
                GSH_DEBUG("Clean up http response");
                clean_up();
            }

            void clean_up()
            {
                // if (body != nullptr)
                //     free(body);
                if (status_code != nullptr)
                    free(status_code);
                if (status_text != nullptr)
                    free(status_text);
                if (request_headers != nullptr)
                    free(request_headers);
                if (response_headers != nullptr)
                    free(response_headers);
            }
        };

    public:
        HttpService();
        ~HttpService() 
        {
            GSH_TRACE("Http service destroied");
        };
        HttpService(HttpService &other) = delete;
        void operator=(const HttpService &) = delete;

        static HttpService& GetInstance();
        bool init(const char* ssid, const char* password, nsapi_security security=NSAPI_SECURITY_WPA_WPA2);

        SharedPtr<HttpResponse> http_request(char* http_headers, SharedPtr<ParsedUrl> purl);

        SharedPtr<HttpResponse> http_get(const char *url, char *custom_headers);
        SharedPtr<HttpResponse> http_post(const char *url, char *custom_headers, char *post_data);

    private:
        SharedPtr<Socket> m_Socket;
    };

}