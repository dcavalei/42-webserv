//
// Created by dcavalei on 28-07-2022.
//

#ifndef INC_42_WEBSERV_REQUEST_HPP
#define INC_42_WEBSERV_REQUEST_HPP

#include "http/http.hpp"
#include "http/RequestLine.hpp"
#include "http/Field.hpp"
#include <ext/stdio_filebuf.h>

namespace http {

    class Request {
    public:
        Request() {

        }

        // returns 501 || 505 || SUCCESS;
        int checkRequestLine() {
            size_t i = -1;
            while (http::methods[++i] != 0 && http::methods[i] != _requestLine.method);
            if (http::methods[i] == NULL) {
                return 501;
            } else if (_requestLine.version != "HTTP/1.1") {
                return 505;
            }

            return SUCCESS;
        }

        int parse(int eventFd) {
            FILE *f = fdopen(eventFd, "r");
            if (!f) {
                return 500;
            }
            __gnu_cxx::stdio_filebuf<char> fileBuffer(f, std::ios::in);
            std::istream is(&fileBuffer);
            std::string line;

            int ret;

            std::getline(is, line);
            if ((ret = setRequestLine(line)) != SUCCESS ||
                (ret = checkRequestLine()) != SUCCESS) {
                LOG_WARNING("Bad Request Line, error " << ret);
                return ret;
            }

            while (std::getline(is, line) && line != "\r" && !line.empty()) {
                addField(line);
            }

            // Only GET POST DELETE are handled, body not required
            if (_requestLine.method == http::methods[GET] ||
                _requestLine.method == http::methods[DELETE]) {
                return SUCCESS;
            } else if (_requestLine.method != http::methods[POST]) {
                return 400;
            }

            std::pair<std::set<Field>::const_iterator, bool> pair = findField("Content-Length");
            int size = std::atoi(pair.first->value.data());
            if (pair.second) {
                _body.reserve(size);
            } else {
                LOG_WARNING("Content-Length not specified, error " << 411);
                return 411;
            }

            LOG_DEBUG(_body);
            while (std::getline(is, line)) {
                LOG_DEBUG(line);
                _body += line += '\n';
            }
            return SUCCESS;
        }

        const RequestLine &getRequestLine() const {
            return _requestLine;
        }

        const std::set<Field> &getFields() const {
            return _fields;
        }

        const std::string &getBody() const {
            return _body;
        }

        Request &changeUriPathTranslated(const std::string &str) {
            _requestLine.uri.pathTranslated = str;
            return *this;
        }

        std::pair<std::set<Field>::const_iterator, bool> findField(std::string const &name) const {
            std::set<Field>::const_iterator target = _fields.find(Field(name.data()));

            return std::make_pair(target, (target != _fields.end()));
        }

    private:

        int setRequestLine(std::string const &line) {
            char b[3][__1KB * 4];
            if (std::sscanf(line.data(), "%s %s %s\r", b[0], b[1], b[2]) != 3) {
                LOG_ERROR("Unable to parse Request Line");
                return 400;
            }
            _requestLine.method = b[0];
            _requestLine.uri = b[1];
            _requestLine.version = b[2];

            LOG_DEBUG("RequestLine(" << _requestLine.method << ' ' << _requestLine.uri.operator std::string() << ' '
                                     << _requestLine.version << ')');
            return 0;
        }

        bool isValidMethod(std::string const &m) {
            size_t i = -1;

            while (http::methods[++i] && m != http::methods[i]);

            return http::methods[i] != NULL;
        }


        bool addField(std::string const &line) {
            char b[2][__1KB * 8];
            Field f;
            if (std::sscanf(line.data(), "%[^:]: %[^\r]", b[0], b[1]) != 2) {
                LOG_ERROR("Unable to parse Field " << line);
                return false;
            }
            f.name = b[0];
            f.value = b[1];
            _fields.insert(f);
            LOG_DEBUG("Field(" << f.name << ": " << f.value << ")");
            return true;
        }

    private:
        RequestLine _requestLine;
        std::set<Field> _fields;
        std::string _body;
    };

} // http

#endif //INC_42_WEBSERV_REQUEST_HPP
