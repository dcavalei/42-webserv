#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <netinet/in.h>
#include <cstring>
#include <fcntl.h>
#include <arpa/inet.h>
#include <fstream>
#include <sstream>
#include "util/util.hpp"

#include "net/Socket.hpp"
#include "net/TCPSocket.hpp"
#include "srv/ServerConfig.hpp"
#include "app/app.hpp"

#define MAX_EVENTS 1000
#define PORT 8080

# include "app/app.hpp"

//volatile bool app::app::on = true;
#include <ext/stdio_filebuf.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
int main(int ac, char *av[]) {

    std::cout << &http::methods << std::endl;
    {
        app::Parser *parser = new app::Parser();
        net::Epoll *epoll = new net::Epoll();

        app::app application(ac,
                             av,
                             "srcs/server.conf",
                             parser,
                             epoll);

        if (!application.load()) {
            LOG_ERROR("Shutting down...");
        } else {
            LOG("Application starting...");
            application.start();
            return EXIT_SUCCESS;
        }
        delete parser;
        delete epoll;
        return EXIT_FAILURE;
    }
//
//
//
//    /* ---------------------------------- */
//    std::cout << srv::statusCodeToString((srv::statusCode) 301) << std::endl;
//
//    struct epoll_event ev;
//    struct epoll_event events[MAX_EVENTS];
//    int conn_sock, nfds, epollfd;
//
//    bzero(&ev, sizeof(ev));
//    bzero(&events, sizeof(events));
//
//    Body body("pages/helloWorld.html");
//
//    std::stringstream ss;
//    std::string slen;
//    int len = (int) body.build().length();
//    ss << len;
//    ss >> slen;
//
//    Header header = Header("HTTP/1.1", "200", "OK");
//
//    header
//            .addField(Field("Content-Type")
//                              .addValue("text/html")
//            )
//            .addField(Field("Content-Length")
//                              .addValue(slen)
//            );
//
//    std::string tmp(header.build() += body.build());
//
//
//    const char *response = tmp.c_str();
//
//    net::TCPSocket server_fd;
//
//    server_fd.init();
//
//    server_fd
//            .bind("0.0.0.0", PORT)
//            .listen(MAX_EVENTS);
//
//    epollfd = epoll_create(MAX_EVENTS);
//    if (epollfd == -1) {
//        perror("epoll_create1");
//        exit(EXIT_FAILURE);
//    }
//
//    ev.events = EPOLLIN;
//    ev.data.fd = server_fd;
//    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
//        perror("epoll_ctl: server_fd");
//        exit(EXIT_FAILURE);
//    }
//
//    while (true) {
//        util::Log("Epoll", "Start");
//        // TODO: print number of fds monitored
//        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
//        if (nfds == -1) {
//            perror("epoll_wait");
//            exit(EXIT_FAILURE);
//        }
//        std::cout << "number of events: " << nfds << std::endl;
//
//        for (int n = 0; n < nfds; ++n) {
//            if (events[n].events & EPOLLRDHUP) {
//                std::cout << "client closed connection" << std::endl;
//                // remove from interest list
//                if (epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, NULL)) {
//                    perror("epoll_ctl: close");
//                }
//                // close fd
//                std::cout << close(events[n].data.fd) << std::endl;
//
//                /**
//                 * using close(events[n].data.fd) will remove that fd from the interest list, (I'm not sure)
//                 */
//                continue;
//            }
//            if (events[n].data.fd == server_fd) { // if new connection
//                conn_sock = server_fd.accept();
//
//                //set non blocking
//                if (fcntl(conn_sock, F_SETFL, O_NONBLOCK) == -1) {
//                    perror("calling fcntl");
//                }
//
//                ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
//                ev.data.fd = conn_sock;
//                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
//                              &ev) == -1) {
//                    perror("epoll_ctl: conn_sock");
//                    exit(EXIT_FAILURE);
//                }
//            } else { // for each events[n].data.fd - do something
//                int fd = events[n].data.fd;
//                std::cout << "fd: " << fd << std::endl;
//                char buffer[24 * 1024] = {0};
//                ssize_t readLen = recv(fd, buffer, sizeof(buffer), 0);
//                if (readLen <= 0) {
//                    util::Log("recv", "Something's wrong", true);
//                    close(fd);
//                    continue;
//                }
//                std::cout << "--------------------------------[START LOG]---------------------------------\n";
//                std::cout << "----------------------------------REQUEST-----------------------------------\n" << buffer
//                          << std::endl;
//                if (send(conn_sock, response, strlen(response), 0) < 0) {
//                    std::cerr << "An error occurred while trying to send the response" << std::endl;
//                    continue;
//                }
//                std::cout << "----------------------------------RESPONSE-----------------------------------\n"
//                          << response << std::endl;
//                std::cout << "---------------------------------[END LOG]-----------------------------------\n";
//            }
//        }
//    }
}
