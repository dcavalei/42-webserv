#ifndef INC_42_WEBSERV_SERVER_HPP
#define INC_42_WEBSERV_SERVER_HPP

#include "srv/ServerConfig.hpp"

namespace srv {

    class Server {
    public:
        Server(ServerConfig const &serverConfig) : _serverConfig(serverConfig) {}

        ~Server() {}

        void initSocket() {
            _socket.init();
            _socket
                    .bind("0.0.0.0", _serverConfig.listen)
                    .listen(1000);
        }

        ServerConfig const &getServerConfig() const { return _serverConfig; }

        net::TCPSocket const &getSocket() const { return _socket; }

        Server &setSocket(net::TCPSocket socket) {
            _socket = socket;
            return *this;
        }

        bool operator<(Server const &other) const { return _serverConfig < other._serverConfig; }

    private:
        ServerConfig _serverConfig;
        net::TCPSocket _socket;
    };

}
#endif //INC_42_WEBSERV_SERVER_HPP
