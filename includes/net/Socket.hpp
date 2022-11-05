#ifndef INC_42_WEBSERV_SOCKET_HPP
#define INC_42_WEBSERV_SOCKET_HPP

#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

namespace net {

    /**
     * @brief General purpose Socket object.
     *
     * Responsible for creating an endpoint for communication, it holds a file descriptor that refers to that endpoint.
     * <a href="https://man7.org/linux/man-pages/man2/socket.2.html">See more...</a>
     *
     * @attention
     * Requires init() in order to create a socket, and close() to close the socket **fd**. There's no automatic
     * behaviour to avoid errors using *C++* containers.
     *
     * @param domain
     * The domain argument specifies a communication domain. This selects the protocol family which will be used for
     * communication. These families are defined in <sys/socket.h>.
     *
     * @param type
     * The Socket has the indicated type, which specifies the communication semantics.
     *
     * @param protocol
     * The protocol specifies a particular protocol to be used with the Socket.
     *
     * @param fd
     * The fd is a file descriptor for the new Socket.
     */
    class Socket {
    public:

        /**
         * @brief Constructor.
         *
         * Creates a Socket object, setting the internal *data types* according to the constructor parameters.
         *
         * @param domain
         * The domain argument specifies a communication domain. This selects the protocol family which will be used
         * for communication. These families are defined in <sys/socket.h>.
         *
         * @param type
         * The Socket has the indicated type, which specifies the communication semantics.
         *
         * @param protocol
         * The protocol specifies a particular protocol to be used with the Socket.
         */
        Socket(int domain, int type, int protocol) : domain(domain), type(type), protocol(protocol), fd(-1) {}

        virtual ~Socket() {};

        /**
         * @brief Initialize Socket.
         *
         * Uses <a href="https://man7.org/linux/man-pages/man2/socket.2.html">socket()</a> system call to initialize
         * the Socket according to *domain*, *type* and *protocol* and saves the return value in **fd**. If init() is
         * used successfully, close() must be used to close the corresponding **fd** once the socket is no longer
         * needed.
         *
         * @attention Throw ***std::runtime_error*** on error.
         */
        void init() {
            if ((fd = ::socket(domain, type, protocol)) < 0) {
                throw std::runtime_error(("init: " + std::string(std::strerror(errno))).c_str());
            }
        }

        /**
         * @brief Close Socket.
         *
         * Uses <a href="https://man7.org/linux/man-pages/man2/close.2.html">close()</a> system call to close the
         * Socket's **fd** and than set it to *-1*.
         *
         * @attention Throw ***std::runtime_error*** on error.
         */
        void close() {
            if (::close(fd) < 0) {
                throw std::runtime_error(("close: " + std::string(std::strerror(errno))).c_str());
            }
            fd = -1;
        }

        /**
         * @return fd
         */
        operator int() const { return fd; }

        /**
         *  @return domain
         */
        int getDomain() const { return domain; }

        /**
         *  @return type
         */
        int getType() const { return type; }

        /**
         *  @return protocol
         */
        int getProtocol() const { return protocol; }

        /**
         *  @return fd
         */
        int getFd() const { return fd; }

    protected:

        /**
         * @brief Default constructor
         *
         * Uses Socket(int domain, int type, int protocol) to create an obsolete Socket. This constructor is
         * *protected* i.e. it can only be accessed by itself or by a child class.
         */
        Socket() : domain(-1), type(-1), protocol(-1), fd(-1) {};

        int domain;
        int type;
        int protocol;
        int fd;
    };
};

#endif //INC_42_WEBSERV_SOCKET_HPP
