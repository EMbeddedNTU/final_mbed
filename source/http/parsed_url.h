#pragma once

#include "mbed.h"
#include "platform/SharedPtr.h"
#include "logger.h"
#include <stdio.h>
#include <ctype.h>

namespace GSH {
    
    struct ParsedUrl 
    {
        char *uri;					/* mandatory */
        char *scheme;               /* mandatory */
        char *host;                 /* mandatory */
        char *ip; 					/* mandatory */
        char *port;                 /* optional */
        char *path;                 /* optional */
        char *query;                /* optional */
        char *fragment;             /* optional */
        char *username;             /* optional */
        char *password;             /* optional */

        static SharedPtr<ParsedUrl> create(const char *url)
        {
            SharedPtr<ParsedUrl> ptr(new ParsedUrl);
            if (!ptr->Parse(url))
            {
                GSH_ERROR("Url can't be parsed");
                return nullptr;
            }
            if(!ptr->isInitialized())
            {
                GSH_ERROR("ParsedUrl object is not initialized");
                return nullptr;
            }
            return ptr;
        }

        ParsedUrl() 
        {
            scheme = NULL;
            host = NULL;
            port = NULL;
            path = NULL;
            query = NULL;
            fragment = NULL;
            username = NULL;
            password = NULL;
        }

        ~ParsedUrl()
        {
            clean_up();
        }


        bool isInitialized()
        {
            return (uri != NULL) && (scheme != NULL) && (host != NULL) && (ip != NULL);
        }

        bool Parse(const char *url)
        {
            
            /* Define variable */
            const char *tmpstr;
            const char *curstr;
            int len;
            int i;
            int userpass_flag;
            int bracket_flag;

            curstr = url;

            /*
            * <scheme>:<scheme-specific-part>
            * <scheme> := [a-z\+\-\.]+
            *             upper case = lower case for resiliency
            */
            /* Read scheme */
            tmpstr = strchr(curstr, ':');
            if ( NULL == tmpstr ) 
            {
                GSH_ERROR("Error reading url scheme no ':'");
                return false;
            }

            /* Get the scheme length */
            len = tmpstr - curstr;

            /* Check restrictions */
            for ( i = 0; i < len; i++ ) 
            {
                if (is_scheme_char(curstr[i]) == 0) 
                {
                    GSH_ERROR("Invalid url format");
                    return false;
                }
            }

            /* Copy the scheme to the storage */
            scheme = (char*)malloc(sizeof(char) * (len + 1));
            if ( NULL == scheme ) 
            {
                GSH_ERROR("Malloc scheme failed");
                clean_up();
                return false;
            }

            (void)strncpy(scheme, curstr, len);
            scheme[len] = '\0';

            /* Make the character to lower if it is upper case. */
            for ( i = 0; i < len; i++ ) 
            {
                scheme[i] = tolower(scheme[i]);
            }

            /* Skip ':' */
            tmpstr++;
            curstr = tmpstr;

            /*
            * //<user>:<password>@<host>:<port>/<url-path>
            * Any ":", "@" and "/" must be encoded.
            */
            /* Eat "//" */
            for ( i = 0; i < 2; i++ ) 
            {
                if ( '/' != *curstr ) 
                {
                    GSH_ERROR("Expected '/'");
                    clean_up();
                    return false;
                }
                curstr++;
            }

            /* Check if the user (and password) are specified. */
            userpass_flag = 0;
            tmpstr = curstr;
            while ( '\0' != *tmpstr ) 
            {
                if ( '@' == *tmpstr ) 
                {
                    /* Username and password are specified */
                    userpass_flag = 1;
                    break;

                } 
                else if ( '/' == *tmpstr ) 
                {
                    /* End of <host>:<port> specification */
                    userpass_flag = 0;
                    break;
                }
                tmpstr++;
            }

            /* User and password specification */
            tmpstr = curstr;
            if ( userpass_flag ) 
            {
                /* Read username */
                while ( '\0' != *tmpstr && ':' != *tmpstr && '@' != *tmpstr ) 
                {
                    tmpstr++;
                }
                len = tmpstr - curstr;
                username = (char*)malloc(sizeof(char) * (len + 1));
                if ( NULL == username ) 
                {
                    GSH_ERROR("Malloc username failed");
                    clean_up();
                    return false;
                }
                (void)strncpy(username, curstr, len);
                username[len] = '\0';

                /* Proceed current pointer */
                curstr = tmpstr;
                if ( ':' == *curstr ) 
                {
                    /* Skip ':' */
                    curstr++;
                    
                    /* Read password */
                    tmpstr = curstr;
                    while ( '\0' != *tmpstr && '@' != *tmpstr ) 
                    {
                        tmpstr++;
                    }
                    len = tmpstr - curstr;
                    password = (char*)malloc(sizeof(char) * (len + 1));
                    if ( NULL == password ) 
                    {
                        GSH_ERROR("Malloc password failed");
                        clean_up();
                        return false;
                    }
                    (void)strncpy(password, curstr, len);
                    password[len] = '\0';
                    curstr = tmpstr;
                }
                /* Skip '@' */
                if ( '@' != *curstr ) 
                {
                    GSH_ERROR("Expected '@'");
                    clean_up();
                    return false;
                }
                curstr++;
            }

            if ( '[' == *curstr ) 
            {
                bracket_flag = 1;
            } 
            else 
            {
                bracket_flag = 0;
            }
            /* Proceed on by delimiters with reading host */
            tmpstr = curstr;
            while ( '\0' != *tmpstr ) {
                if ( bracket_flag && ']' == *tmpstr )
                {
                    /* End of IPv6 address. */
                    tmpstr++;
                    break;
                } 
                else if ( !bracket_flag && (':' == *tmpstr || '/' == *tmpstr) ) 
                {
                    /* Port number is specified. */
                    break;
                }
                tmpstr++;
            }
            len = tmpstr - curstr;
            host = (char*)malloc(sizeof(char) * (len + 1));
            if ( NULL == host || len <= 0 ) 
            {
                GSH_ERROR("Malloc host failed");
                clean_up();
                return false;
            }
            (void)strncpy(host, curstr, len);
            host[len] = '\0';
            curstr = tmpstr;

            /* Is port number specified? */
            if ( ':' == *curstr ) 
            {
                curstr++;
                /* Read port number */
                tmpstr = curstr;
                while ( '\0' != *tmpstr && '/' != *tmpstr ) 
                {
                    tmpstr++;
                }
                len = tmpstr - curstr;
                port = (char*)malloc(sizeof(char) * (len + 1));
                if ( NULL == port ) 
                {
                    GSH_ERROR("Malloc port failed");
                    clean_up();
                    return false;
                }
                (void)strncpy(port, curstr, len);
                port[len] = '\0';
                curstr = tmpstr;
            }
            else
            {
                port = (char*)"80";
            }
            
            // TODO:
            /* Get ip */
            // char *ip = hostname_to_ip(purl->host);
            // purl->ip = ip;
            
            /* Set uri */
            uri = (char*)url;

            /* End of the string */
            if ( '\0' == *curstr ) 
            {
                return true;
            }

            /* Skip '/' */
            if ( '/' != *curstr ) 
            {
                GSH_ERROR("Too many '/'");
                clean_up();
                return false;
            }
            curstr++;

            /* Parse path */
            tmpstr = curstr;
            while ( '\0' != *tmpstr && '#' != *tmpstr  && '?' != *tmpstr ) 
            {
                tmpstr++;
            }
            len = tmpstr - curstr;
            path = (char*)malloc(sizeof(char) * (len + 1));
            if ( NULL == path ) 
            {
                GSH_ERROR("Malloc path failed");
                clean_up();
                return false;
            }
            (void)strncpy(path, curstr, len);
            path[len] = '\0';
            curstr = tmpstr;

            /* Is query specified? */
            if ( '?' == *curstr ) 
            {
                /* Skip '?' */
                curstr++;
                /* Read query */
                tmpstr = curstr;
                while ( '\0' != *tmpstr && '#' != *tmpstr ) 
                {
                    tmpstr++;
                }
                len = tmpstr - curstr;
                query = (char*)malloc(sizeof(char) * (len + 1));
                if ( NULL == query ) 
                {
                    GSH_ERROR("Malloc query failed");
                    clean_up();
                    return false;
                }
                (void)strncpy(query, curstr, len);
                query[len] = '\0';
                curstr = tmpstr;
            }

            /* Is fragment specified? */
            if ( '#' == *curstr ) 
            {
                /* Skip '#' */
                curstr++;
                /* Read fragment */
                tmpstr = curstr;
                while ( '\0' != *tmpstr ) 
                {
                    tmpstr++;
                }
                len = tmpstr - curstr;
                fragment = (char*)malloc(sizeof(char) * (len + 1));
                if ( NULL == fragment )
                {
                    GSH_ERROR("Malloc fragment failed");
                    clean_up();
                    return false;
                }
                (void)strncpy(fragment, curstr, len);
                fragment[len] = '\0';
                curstr = tmpstr;
            }
            return true;
        }

        private:
            int is_scheme_char(int c)
            {
                return (!isalpha(c) && '+' != c && '-' != c && '.' != c) ? 0 : 1;
            }

            void clean_up()
            {
                if ( NULL != scheme ) free(scheme);
                if ( NULL != host ) free(host);
                if ( NULL != port ) free(port);
                if ( NULL != path )  free(path);
                if ( NULL != query ) free(query);
                if ( NULL != fragment ) free(fragment);
                if ( NULL != username ) free(username);
                if ( NULL != password ) free(password);
            }
    };
}