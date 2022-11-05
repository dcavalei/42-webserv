#ifndef INC_42_WEBSERV_SERVERCONFIG_HPP
#define INC_42_WEBSERV_SERVERCONFIG_HPP

#include <deque>
#include <set>
#include <string>
#include <map>
#include <cstdlib>
#include "util/util.hpp"
#include "util/macros.h"

namespace srv {
    struct redirect {
        int statusCode;
        std::string url;
    };

    struct location {
        location(char const *path = "/")
                : path(path), root(path), autoIndex(false), cgi(false) {
            const char *buffer[3] = {
                    "GET",
                    "POST",
                    "DELETE",
            };
            allowedMethods.insert(buffer, buffer + 3);
        }

        bool operator<(const location &other) const {
            return path < other.path;
        }

        std::string path;
        std::string root;
        bool autoIndex;
        std::set<std::string> allowedMethods;
        util::optional<redirect> redirect;
        bool cgi;
    };

    struct ServerConfig {
        ServerConfig(std::string const &name = "localhost", int listen = -1) :
                serverName(name),
                listen(listen),
                client_max_body_size(10 * __1MB),
                locations() {}

        bool operator<(const ServerConfig &other) const {
            return (serverName < other.serverName || listen < other.listen);
        }

        std::string serverName;
        int listen;
        size_t client_max_body_size;
        std::set<location> locations;
    };
}

#endif //INC_42_WEBSERV_SERVERCONFIG_HPP
