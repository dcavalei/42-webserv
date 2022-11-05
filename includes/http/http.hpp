//
// Created by dcavalei on 07-08-2022.
//

#ifndef INC_42_WEBSERV_HTTP_HPP
#define INC_42_WEBSERV_HTTP_HPP

namespace http {

    char const *const methods[] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", NULL};
    enum method {
        GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE
    };

} // http

#endif //INC_42_WEBSERV_HTTP_HPP
