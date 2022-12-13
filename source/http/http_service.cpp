#include "core/pch.h"
#include <cstdint>
#include "http_service.h"
#include "string_utils.h"
#include <sstream>

namespace GSH {

    HttpService& HttpService::GetInstance()
    {
        static HttpService s_HttpService;
        return s_HttpService;
    }

    HttpService::HttpService()
    {

    }

    bool HttpService::init(const char* ssid, const char* password, nsapi_security security)
    {
        GSH_INFO("init http service");

        m_Socket = Socket();

        if (!m_Socket.init()) 
        {
            GSH_INFO("Socket init failed");
            return false;
        }
    
        if (!m_Socket.wifi_connect(ssid, password, security))
        {
            GSH_INFO("init http service");
            return false;
        }

        return true;
    }


    /*
	Makes a HTTP request and returns the response
    */
    HttpService::HttpResponse* HttpService::http_request(char* http_headers, parsed_url* purl)
    {
        /* Parse url */
        if(purl == NULL)
        {
            printf("Unable to parse url");
            return NULL;
        }

        /* Allocate memeory for htmlcontent */
        HttpResponse *hresp = (HttpResponse*)malloc(sizeof(struct HttpResponse));
        if(hresp == NULL)
        {
            printf("Unable to allocate memory for htmlcontent.");
            return NULL;
        }
        hresp->body = NULL;
        hresp->request_headers = NULL;
        hresp->response_headers = NULL;
        hresp->status_code = NULL;
        hresp->status_text = NULL;

        char buf[BUFSIZ+1];

        // TODO
        stringstream strValue;
        strValue << purl->port;
        unsigned int portNumber;
        strValue >> portNumber;
        GSH_INFO("port %d", portNumber);
        m_Socket.connect(purl->host, portNumber);

        /* Send headers to server */
        m_Socket.send(http_headers, strlen(http_headers));

        /* Recieve into response*/
        char *response = (char*)malloc(0);
        char BUF[BUFSIZ];
        size_t recived_len = 0;
        while((recived_len = m_Socket.recv_chunk(BUF, BUFSIZ-1)) > 0)
        {
            BUF[recived_len] = '\0';
            response = (char*)realloc(response, strlen(response) + strlen(BUF) + 1);
            sprintf(response, "%s%s", response, BUF);
        }
        if (recived_len < 0)
        {
            free(http_headers);
            printf("Unabel to recieve");
            return NULL;
        }

        /* Reallocate response */
        response = (char*)realloc(response, strlen(response) + 1);
       
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

        /* Parse response headers */
        char *headers = get_until(response, "\r\n\r\n");
        hresp->response_headers = headers;

        /* Assign request headers */
        hresp->request_headers = http_headers;

        /* Assign request url */
        hresp->request_uri = purl;

        /* Parse body */
        char *body = strstr(response, "\r\n\r\n");
        body = str_replace("\r\n\r\n", "", body);
        hresp->body = body;

        /* Return response */
        return hresp;
    }

    

    /*
        Makes a HTTP GET request to the given url
    */
    HttpService::HttpResponse* HttpService::http_get(char *url, char *custom_headers)
    {
        /* Parse url */
        struct parsed_url *purl = ParseUrlUtil::parse_url(url);
        if(purl == NULL)
        {
            printf("Unable to parse url");
            return NULL;
        }

        /* Declare variable */
        char *http_headers = (char*)malloc(1024);

        /* Build query/headers */
        if(purl->path != NULL)
        {
            if(purl->query != NULL)
            {
                sprintf(http_headers, "GET /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->query, purl->host);
            }
            else
            {
                sprintf(http_headers, "GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->host);
            }
        }
        else
        {
            if(purl->query != NULL)
            {
                sprintf(http_headers, "GET /?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->query, purl->host);
            }
            else
            {
                sprintf(http_headers, "GET / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->host);
            }
        }

        /* Handle authorisation if needed */
        if(purl->username != NULL)
        {
            /* Format username:password pair */
            char *upwd = (char*)malloc(1024);
            sprintf(upwd, "%s:%s", purl->username, purl->password);
            upwd = (char*)realloc(upwd, strlen(upwd) + 1);

            /* Base64 encode */
            char *base64 = base64_encode(upwd);

            /* Form header */
            char *auth_header = (char*)malloc(1024);
            sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
            auth_header = (char*)realloc(auth_header, strlen(auth_header) + 1);

            /* Add to header */
            http_headers = (char*)realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
            sprintf(http_headers, "%s%s", http_headers, auth_header);
        }

        /* Add custom headers, and close */
        if(custom_headers != NULL)
        {
            sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
        }
        else
        {
            sprintf(http_headers, "%s\r\n", http_headers);
        }
        http_headers = (char*)realloc(http_headers, strlen(http_headers) + 1);

        /* Make request and return response */
        HttpResponse *hresp = http_request(http_headers, purl);

        // /* Handle redirect */
        // return handle_redirect_get(hresp, custom_headers);
        return hresp;
    }

    /*
	Makes a HTTP POST request to the given url
    */
    HttpService::HttpResponse* HttpService::http_post(char *url, char *custom_headers, char *post_data)
    {
        /* Parse url */
        struct parsed_url *purl = ParseUrlUtil::parse_url(url);
        if(purl == NULL)
        {
            printf("Unable to parse url");
            return NULL;
        }

        /* Declare variable */
        char *http_headers = (char*)malloc(1024);

        /* Build query/headers */
        if(purl->path != NULL)
        {
            if(purl->query != NULL)
            {
                sprintf(http_headers, "POST /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->path, purl->query, purl->host, strlen(post_data));
            }
            else
            {
                sprintf(http_headers, "POST /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->path, purl->host, strlen(post_data));
            }
        }
        else
        {
            if(purl->query != NULL)
            {
                sprintf(http_headers, "POST /?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->query, purl->host, strlen(post_data));
            }
            else
            {
                sprintf(http_headers, "POST / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->host, strlen(post_data));
            }
        }

        /* Handle authorisation if needed */
        if(purl->username != NULL)
        {
            /* Format username:password pair */
            char *upwd = (char*)malloc(1024);
            sprintf(upwd, "%s:%s", purl->username, purl->password);
            upwd = (char*)realloc(upwd, strlen(upwd) + 1);

            /* Base64 encode */
            char *base64 = base64_encode(upwd);

            /* Form header */
            char *auth_header = (char*)malloc(1024);
            sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
            auth_header = (char*)realloc(auth_header, strlen(auth_header) + 1);

            /* Add to header */
            http_headers = (char*)realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
            sprintf(http_headers, "%s%s", http_headers, auth_header);
        }

        if(custom_headers != NULL)
        {
            sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
            sprintf(http_headers, "%s\r\n%s", http_headers, post_data);
        }
        else
        {
            sprintf(http_headers, "%s\r\n%s", http_headers, post_data);
        }
        http_headers = (char*)realloc(http_headers, strlen(http_headers) + 1);

        /* Make request and return response */
        HttpResponse *hresp = http_request(http_headers, purl);

        /* Handle redirect */
        // return handle_redirect_post(hresp, custom_headers, post_data);
        return hresp;
    }


}
