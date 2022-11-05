//
// Created by dcavalei on 28-07-2022.
//

#ifndef INC_42_WEBSERV_REQUESTLINE_HPP
#define INC_42_WEBSERV_REQUESTLINE_HPP

#include <string>
#include "http/Uri.hpp"

namespace http {

    struct RequestLine {
        std::string method;
        http::Uri uri;
        std::string version;
    };

} // http

#endif //INC_42_WEBSERV_REQUESTLINE_HPP
