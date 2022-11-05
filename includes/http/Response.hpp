//
// Created by dcavalei on 02-08-2022.
//

#ifndef INC_42_WEBSERV_RESPONSE_HPP
#define INC_42_WEBSERV_RESPONSE_HPP

#include "http/StatusLine.hpp"
#include "http/util/getters.hpp"
#include "util/util.hpp"

namespace http {

    class Response {
    public:
        Response() {
        }

        const StatusLine &getStatusLine() const {
            return _statusLine;
        }

        const std::set<Field> &getFields() const {
            return _fields;
        }

        const std::string &getBody() const {
            return _body;
        }

        std::string &getBody() {
            return _body;
        }

        Response &setupStatusLine(int statusCode) {
            _statusLine.version = "HTTP/1.1";
            _statusLine.statusCode = statusCode;
            _statusLine.phrase = http::util::getStatusCodePhrase(statusCode);
            return *this;
        }

        Response &addField(std::string const &name, std::string const &value) {
            Field field(name.data(), value.data());

            if (!_fields.insert(field).second) {
                LOG_ERROR("Failed to insert Field(" << field.name << ": " << field.value << ')');
            } else {
                LOG_DEBUG("Field(" << field.name << ": " << field.value << ") added to Response");
            }
            return *this;
        }

        Response &addField(std::string const &line) {
            char b[2][__1KB * 8];
            Field f;
            if (std::sscanf(line.data(), "%[^:]: %[^\r]", b[0], b[1]) != 2) {
                LOG_ERROR("Unable to parse Field: " << line);
                return *this;
            }
            f.name = b[0];
            f.value = b[1];
            _fields.insert(f);
            LOG_DEBUG("Field(" << f.name << ": " << f.value << ")");
            return *this;
        }

        Response &setBodyFromFile(std::string const &file) {
            std::ifstream t(file.data());
            std::stringstream buffer;
            buffer << t.rdbuf();
            buffer.str().swap(_body);
            return *this;
        }

        Response &setBodyFromString(std::string const &string) {
            _body = string;
            return *this;
        }

        // TODO: improve performance pls. xd
        void sendResponse(int eventFd) const {
            std::string tmp;

            tmp += _statusLine.statusLineToHttpLine();
            std::set<http::Field>::iterator itb = _fields.begin();
            std::set<http::Field>::iterator ite = _fields.end();

            for (; itb != ite; ++itb) {
                tmp += itb->fieldToHttpLine();
            }
            tmp += "\r\n";
            tmp += _body;
            LOG_DEBUG(tmp);
            ::util::sendAll(eventFd, tmp.data(), tmp.length());
        }

    private:
        StatusLine _statusLine;
        std::set<Field> _fields;
        std::string _body;
    };

} // http

#endif //INC_42_WEBSERV_RESPONSE_HPP
