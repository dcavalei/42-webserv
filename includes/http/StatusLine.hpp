//
// Created by dcavalei on 02-08-2022.
//

#ifndef INC_42_WEBSERV_STATUSLINE_HPP
#define INC_42_WEBSERV_STATUSLINE_HPP

#include <string>

namespace http {

    struct StatusLine {

        std::string statusLineToHttpLine() const {
            std::stringstream str;
            str << version << ' ' << statusCode << ' ' << phrase << "\r\n";
            return str.str();
        }

        std::string version;
        int statusCode;
        std::string phrase;
    };

} // http

#endif //INC_42_WEBSERV_STATUSLINE_HPP
