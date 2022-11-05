#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>

#define THIS_VALUE_IS_USELESS_BUT_MUST_BE_BIGGER_THAN_ZERO 10
#define MAX_EPOLL_EVENTS_PER_RUN 1000
#define EPOLL_RUN_TIMEOUT 1000
#define MAX_EVENTS 10
#define PORT 8080

int main(int argc, char const *argv[])
{
    struct epoll_event ev, events[MAX_EVENTS];
    int conn_sock, nfds, epollfd;

    int server_fd, new_socket;
    long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Only this line has been changed. Everything is same.
    char *hello = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 88\n\n<html>\n<body>\n<h1>Hello, World!</h1>\n</body>\n</html>\n";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    memset(address.sin_zero, '\0', sizeof address.sin_zero);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    epollfd = epoll_create(THIS_VALUE_IS_USELESS_BUT_MUST_BE_BIGGER_THAN_ZERO);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_fd, &ev) == -1)
    {
        perror("epoll_ctl: server_fd");
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        std::cout << "number of requests: " << nfds << std::endl;

        for (int n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == server_fd)
            {
                conn_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
                if (conn_sock == -1)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                // set non blocking
                if (fcntl(conn_sock, F_SETFL, fcntl(conn_sock, F_GETFL, 0) | O_NONBLOCK) == -1)
                {
                    perror("calling fcntl");
                }

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
                              &ev) == -1)
                {
                    perror("epoll_ctl: conn_sock");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                char buffer[30000] = {0};
                valread = read(events[n].data.fd, buffer, 30000);
                printf("%s\n", buffer);
                write(events[n].data.fd, hello, strlen(hello));
                printf("------------------Hello message sent-------------------\n");
                close(events[n].data.fd);
            }
        }
    }
}
