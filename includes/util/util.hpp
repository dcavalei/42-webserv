#ifndef INC_42_WEBSERV_UTIL_HPP
#define INC_42_WEBSERV_UTIL_HPP

#include <iostream>
#include <exception>
#include <ctime>

namespace util {

    void	sendAll(int socketFd, const char* buffer, int length)
    {
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(socketFd, &writefds);

        while (length > 0)
        {
            int availability = select(socketFd + 1, 0, &writefds, 0, 0);
            if (availability <= 0)
            {
                perror("ERROR");
                exit(1);
            }
            if (availability != 1)
            {
                continue;
            }
            int sent = send(socketFd, buffer, length, 0);
            if (sent < 0)
            {
                perror("ERROR");
                exit(1);
            }

            buffer += sent;
            length -= sent;
        }
    }

    uint64_t randomNumber() {
        static bool isSeedSet = false;
        if (!isSeedSet) {
            std::srand(std::time(0));
        }
        return std::rand() * std::rand();
    }

    std::string size_tToString(size_t i) {
        std::stringstream ss;

        ss << i;
        return ss.str();
    }

    inline std::string &ltrim(std::string &string) {
        std::size_t start = string.find_first_not_of(" \n\r\t\f\v");
        return (start == std::string::npos) ? (string = "") : (string = string.substr(start));
    }

    inline std::string &rtrim(std::string &string) {
        std::size_t end = string.find_last_not_of(" \n\r\t\f\v");
        return ((string = string.substr(0, end + 1)));
    }

    inline std::string &trim(std::string &string) {
        return (rtrim(ltrim(string)));
    }

    // bool to string
    inline std::string btos(bool b) {
        return b ? "true" : "false";
    }

    template<class T>
    class optional {
    public:
        typedef T value_type;

        optional() : _ptr(NULL) {}

        optional(value_type const &value) : _ptr(NULL) {
            _ptr = new value_type(value);
        }

        optional(optional const &other) : _ptr(NULL) {
            *this = other;
        }

        ~optional() {
            reset();
        }

        optional &operator=(optional const &other) {
            if (this == &other) {
                return *this;
            }
            reset();
            if (other) {
                _ptr = new T(*other._ptr);
            }
            return *this;
        }

        optional &operator=(T const &other) {
            reset();
            _ptr = new T(other);
            return *this;
        }

        operator bool() const {
            return _ptr != NULL;
        }

        value_type &value() {
            if (!_ptr) {
                throw std::exception();
            }
            return *_ptr;
        }

        const value_type &value() const {
            if (!_ptr) {
                throw std::exception();
            }
            return *_ptr;
        }

        bool has_value() const {
            return _ptr != NULL;
        }

    private:

        void reset() {
            if (_ptr) {
                delete _ptr;
                _ptr = NULL;
            }
        }

        T *_ptr;
    };

}

#endif //INC_42_WEBSERV_UTIL_HPP
