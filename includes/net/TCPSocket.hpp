#ifndef INC_42_WEBSERV_TCPSOCKET_HPP
#define INC_42_WEBSERV_TCPSOCKET_HPP

#include "Socket.hpp"

namespace net {

    class TCPSocket : public Socket {
    public:

        TCPSocket() : Socket(AF_INET, SOCK_STREAM, 0) {}

        TCPSocket &bind(const std::string &ipv4, in_port_t port) {

            address.sin_family = AF_INET;
            address.sin_addr.s_addr = inet_addr(ipv4.c_str());
            address.sin_port = htons(port);

            if (::bind(fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
                throw std::runtime_error(("bind: " + std::string(std::strerror(errno))).c_str());
            }
            return *this;
        }

        TCPSocket& listen(int max_conn_in_queue) {
            if (::listen(fd, max_conn_in_queue) < 0) {
                throw std::runtime_error(("listen: " + std::string(std::strerror(errno))).c_str());
            }
            return *this;
        }

        int accept() {
            int len = sizeof(address);
            int conn_sock;

            if ((conn_sock = ::accept(fd, (struct sockaddr *) &address, (socklen_t *) &len)) < -1) {
                throw std::runtime_error(("accept: " + std::string(std::strerror(errno))).c_str());
            }
            return conn_sock;
        }

        struct sockaddr_in& getAddress() { return address; }

    protected:
        struct sockaddr_in address;

    };
}

#endif //INC_42_WEBSERV_TCPSOCKET_HPP
