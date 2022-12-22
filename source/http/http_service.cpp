#include "http_service.h"
#include "core/pch.h"
#include "string_utils.h"
#include <cstdint>
#include <cstdlib>
#include <sstream>

namespace GSH {

    HttpService &HttpService::GetInstance() 
    {
        static HttpService s_HttpService;
        return s_HttpService;
    }

    HttpService::HttpService() {}

    bool HttpService::init(const char *ssid, const char *password,
                        nsapi_security security) {
        GSH_INFO("Init http service");

        m_Socket = Socket::create();

        if (!m_Socket->init()) {
            GSH_WARN("Socket init failed");
            return false;
        }

        if (!m_Socket->wifi_connect(ssid, password, security)) {
            GSH_WARN("wifi connect failed");
            return false;
        }

        GSH_INFO("init http service successful");
            return true;
    }

    /*
        Makes a HTTP request and returns the response
    */
    SharedPtr<HttpService::HttpResponse> HttpService::http_request(char* http_headers, SharedPtr<ParsedUrl> purl) 
    {
        /* Parse url */
        if (!purl) 
        {
            GSH_ERROR("Unable to parse url");
            return nullptr;
        }

        /* Allocate memeory for htmlcontent */
        SharedPtr<HttpResponse> hresp(new HttpResponse());
        if (!hresp) 
        {
            GSH_ERROR("Unable to allocate memory for htmlcontent.");
            return nullptr;
        }

        char buf[BUFSIZ + 1];

        stringstream strValue;
        strValue << purl->port;
        unsigned int portNumber;
        strValue >> portNumber;
        GSH_TRACE("port %d", portNumber);

        if (!m_Socket->connect(purl->host, portNumber)) 
        {
            GSH_ERROR("Socket connect failed");
            return nullptr;
        }

        /* Send headers to server */
        m_Socket->send(http_headers, strlen(http_headers));

        /* Recieve into response*/
        // char *response = (char *)malloc(0);
        char *response = nullptr;
        char BUF[BUFSIZ];
        size_t recived_len = 0;

        while ((recived_len = m_Socket->recv_chunk(BUF, BUFSIZ - 1)) > 0) {
            BUF[recived_len] = '\0';
            int current_response_len = strlen(response);
            GSH_DEBUG("resp len %d, buf len %d", current_response_len, strlen(BUF));

            if (response == nullptr) {
                response = (char *) malloc(recived_len + 1);
                memcpy(response + current_response_len, BUF, strlen(BUF) + 1);
            } else {
                response = (char *)realloc(response, current_response_len + strlen(BUF) + 1);
                memcpy(response + current_response_len, BUF, strlen(BUF) + 1);
            }            
            GSH_DEBUG("in recv resp %.*s", recived_len, response);
        }
        if (recived_len < 0) {
            free(http_headers);
            GSH_WARN("Unabel to recieve");
            return nullptr;
        }

        /* Reallocate response */
        int response_len = strlen(response) + 1;
        response = (char*)realloc(response, response_len);
        
        GSH_INFO("last b buf %x", BUF[strlen(BUF) + 1] & 0xff);

        /* Parse status code and text */
        char *status_line = get_until(response, "\r\n");
        status_line = str_replace("HTTP/1.1 ", "", status_line);
        char *status_code = str_ndup(status_line, 4);
        status_code = str_replace(" ", "", status_code);
        char *status_text = str_replace(status_code, "", status_line);
        status_text = str_replace(" ", "", status_text);
        hresp->status_code = status_code;
        hresp->status_code_int = atoi(status_code);
        hresp->status_text = status_text;


        // for (int i = 0; i<response_len; i++) {
        //     GSH_DEBUG("b1 resp %x buf %x", response[i] & 0xff, BUF[i] & 0xff);
        // }

                /* Parse body */
        char *body = strstr(response, "\r\n\r\n");
        GSH_DEBUG("resp p %p", response);
        GSH_DEBUG("body %p", body);
        GSH_DEBUG("body %s", body);
        body = str_replace("\r\n\r\n", "", body);
        GSH_DEBUG("body %s", body);
        hresp->body = body;

        /* Parse response headers */
        char *headers = get_until(response, "\r\n\r\n");
        // for (int i = 0; i<response_len; i++) {
        //     GSH_INFO("b2 resp %x buf %x", response[i] & 0xff, BUF[i] & 0xff);
        // }

        hresp->response_headers = headers;

        hresp->request_headers = http_headers;

        hresp->request_uri = purl;

        // for (int i = 0; i<response_len; i++) {
        //     GSH_DEBUG("b5 resp %x buf %x", response[i] & 0xff, BUF[i] & 0xff);
        // }

        // /* Parse body */
        // char *body = strstr(response, "\r\n\r\n");
        // GSH_DEBUG("resp p %p", response);
        // GSH_DEBUG("body %p", body);
        // GSH_DEBUG("body %s", body);
        // body = str_replace("\r\n\r\n", "", body);
        // GSH_DEBUG("body %s", body);
        // hresp->body = body;

        /* Return response */
        return hresp;
    }

    /*
        Makes a HTTP POST request to the given url
    */
    SharedPtr<HttpService::HttpResponse> HttpService::http_get(const char *url, char *custom_headers) 
    {
        /* Parse url */
        SharedPtr<ParsedUrl> purl = ParsedUrl::create(url);
        if (purl == nullptr) 
        {
            GSH_ERROR("Unable to parse url");
            return nullptr;
        }

        /* Declare variable */
        char* http_headers = (char *)malloc(1024);

        /* Build query/headers */
        if (purl->path != NULL) {
            if (purl->query != NULL) {
            sprintf(http_headers,
                    "GET /%s?%s "
                    "HTTP/"
                    "1.1\r\nHost:%s\r\nConnection:close\r\n",
                    purl->path, purl->query, purl->host);
            } else {
            sprintf(http_headers,
                    "GET /%s "
                    "HTTP/"
                    "1.1\r\nHost:%s\r\nConnection:close\r\n",
                    purl->path, purl->host);
            }
        } else {
            if (purl->query != NULL) {
            sprintf(http_headers,
                    "GET /?%s "
                    "HTTP/"
                    "1.1\r\nHost:%s\r\nConnection:close\r\n",
                    purl->query, purl->host);
            } else {
            sprintf(http_headers,
                    "GET / "
                    "HTTP/"
                    "1.1\r\nHost:%s\r\nConnection:close\r\n",
                    purl->host);
            }
        }

        /* Handle authorisation if needed */
        if (purl->username != NULL) {
            /* Format username:password pair */
            char *upwd = (char *)malloc(1024);
            sprintf(upwd, "%s:%s", purl->username, purl->password);
            upwd = (char *)realloc(upwd, strlen(upwd) + 1);

            /* Base64 encode */
            char *base64 = base64_encode(upwd);

            /* Form header */
            char *auth_header = (char *)malloc(1024);
            sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
            auth_header = (char *)realloc(auth_header, strlen(auth_header) + 1);

            /* Add to header */
            http_headers = (char *)realloc(http_headers, strlen(http_headers) +
                                                            strlen(auth_header) + 2);
            sprintf(http_headers, "%s%s", http_headers, auth_header);
        }

        if (custom_headers != NULL) {
            sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
        } else {
            sprintf(http_headers, "%s\r\n", http_headers);
        }
        http_headers = (char *)realloc(http_headers, strlen(http_headers) + 1);

        /* Make request and return response */
        SharedPtr<HttpResponse> hresp = http_request(http_headers, purl);

        /* Handle redirect */
        // return handle_redirect_post(hresp, custom_headers, post_data);

        GSH_TRACE("Http post response returned");
        return hresp;
    }



    /*
        Makes a HTTP POST request to the given url
    */
    SharedPtr<HttpService::HttpResponse> HttpService::http_post(const char *url, char *custom_headers, char *post_data) 
    {
        /* Parse url */
        SharedPtr<ParsedUrl> purl = ParsedUrl::create(url);
        if (purl == nullptr) 
        {
            GSH_ERROR("Unable to parse url");
            return nullptr;
        }

        /* Declare variable */
        char* http_headers = (char *)malloc(1024);

        /* Build query/headers */
        if (purl->path != NULL) {
            if (purl->query != NULL) {
            sprintf(http_headers,
                    "POST /%s?%s "
                    "HTTP/"
                    "1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%"
                    "zu\r\nContent-Type:application/json\r\n",
                    purl->path, purl->query, purl->host, strlen(post_data));
            } else {
            sprintf(http_headers,
                    "POST /%s "
                    "HTTP/"
                    "1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%"
                    "zu\r\nContent-Type:application/json\r\n",
                    purl->path, purl->host, strlen(post_data));
            }
        } else {
            if (purl->query != NULL) {
            sprintf(http_headers,
                    "POST /?%s "
                    "HTTP/"
                    "1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%"
                    "zu\r\nContent-Type:application/json\r\n",
                    purl->query, purl->host, strlen(post_data));
            } else {
            sprintf(http_headers,
                    "POST / "
                    "HTTP/"
                    "1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%"
                    "zu\r\nContent-Type:application/json\r\n",
                    purl->host, strlen(post_data));
            }
        }

        /* Handle authorisation if needed */
        if (purl->username != NULL) {
            /* Format username:password pair */
            char *upwd = (char *)malloc(1024);
            sprintf(upwd, "%s:%s", purl->username, purl->password);
            upwd = (char *)realloc(upwd, strlen(upwd) + 1);

            /* Base64 encode */
            char *base64 = base64_encode(upwd);

            /* Form header */
            char *auth_header = (char *)malloc(1024);
            sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
            auth_header = (char *)realloc(auth_header, strlen(auth_header) + 1);

            /* Add to header */
            http_headers = (char *)realloc(http_headers, strlen(http_headers) +
                                                            strlen(auth_header) + 2);
            sprintf(http_headers, "%s%s", http_headers, auth_header);
        }

        if (custom_headers != NULL) {
            sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
            sprintf(http_headers, "%s\r\n%s", http_headers, post_data);
        } else {
            sprintf(http_headers, "%s\r\n%s", http_headers, post_data);
        }
        http_headers = (char *)realloc(http_headers, strlen(http_headers) + 1);

        /* Make request and return response */
        SharedPtr<HttpResponse> hresp = http_request(http_headers, purl);

        /* Handle redirect */
        // return handle_redirect_post(hresp, custom_headers, post_data);

        GSH_TRACE("Http post response returned");
        return hresp;
    }

} // namespace GSH
