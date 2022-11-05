//
// Created by dcavalei on 28-07-2022.
//

#ifndef INC_42_WEBSERV_URI_HPP
#define INC_42_WEBSERV_URI_HPP

#include <string>
#include <vector>

namespace http {

    struct Uri {
        Uri(char const * const p = "") {
            std::string line(p);
            std::string::size_type pos = line.find('?');

            path = line.substr(0, pos);
            pathTranslated = path;
            if (pos != std::string::npos) {
                query = line.substr(++pos);
            }
        }

        operator std::string() const {
            return data();
        }

        std::string data() const {
            if (!query.empty()) {
                return path + '?' += query;
            }
            return path;
        }

        std::string path;
        std::string pathTranslated;
        std::string query;
    };

} // http

#endif //INC_42_WEBSERV_URI_HPP
