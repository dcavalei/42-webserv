//
// Created by dcavalei on 25-07-2022.
//

#ifndef INC_42_WEBSERV_EPOLL_HPP
#define INC_42_WEBSERV_EPOLL_HPP

#include "util/macros.h"

namespace net {

    class Epoll {
    public:
        Epoll() : _epollFd(-1) {}

        ~Epoll() {
            if (_epollFd != -1) {
                close(_epollFd);
                _epollFd = -1;
            }
        }

        int create() {
            if (_epollFd != -1) {
                LOG_ERROR("Epoll already created");
                return 0;
            }
            _epollFd = epoll_create1(0);
            LOG_DEBUG("Epoll(" << _epollFd << ") created");
            return (_epollFd);
        }

        int add(int fd, uint32_t flags) {
            struct epoll_event ev;

            bzero(&ev, sizeof(ev));
            ev.events = flags;
            ev.data.fd = fd;

            if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) == FAILURE) {
                LOG_ERROR("fd(" << fd << ") failed to add to interest list");
                return FAILURE;
            }

            _interestList.insert(fd);
            LOG_DEBUG("fd(" << fd << ") added to interest list");
            return SUCCESS;
        }

        int remove(int fd) {
            if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) == FAILURE) {
                LOG_ERROR("Failed to remove fd(" << fd << ") from interest list");
                return FAILURE;
            }
            _interestList.erase(fd);
            LOG_DEBUG("fd(" << fd << ") removed from interest list");
            return SUCCESS;
        }

        /**
         * @brief Wait for events
         *
         * @param timeOut specifies the maximum wait time in milliseconds (-1 == infinite).
         * @return std::pair with the number of triggered events (pair.fist) and the array of events (pair.second).
         */
        std::pair<int, struct epoll_event *> wait(int timeOut) {
            return std::make_pair(epoll_wait(_epollFd, events, 1024, timeOut), events);
        }

    private:
        int _epollFd;
        std::set<int> _interestList;
        struct epoll_event events[1024];
    };

} // net

#endif //INC_42_WEBSERV_EPOLL_HPP
