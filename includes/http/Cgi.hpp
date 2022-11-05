//
// Created by dcavalei on 22-08-2022.
//

#ifndef INC_42_WEBSERV_CGI_HPP
#define INC_42_WEBSERV_CGI_HPP

#include "Request.hpp"
#include "Response.hpp"

#include "util/util.hpp"
#include <sys/wait.h>

namespace http {
    class Cgi {
    public:
        Cgi(Request const *rq = NULL, Response *rp = NULL, srv::ServerConfig const *sc = NULL,
            srv::location const *l = NULL) : _request(rq), _response(rp), _serverConfig(sc), _location(l) {
        }

        Cgi const &setRequest(const Request *request) {
            _request = request;
            return *this;
        }

        Cgi const &setResponse(Response *response) {
            _response = response;
            return *this;
        }

        Cgi const &setServerConfig(const srv::ServerConfig *serverConfig) {
            _serverConfig = serverConfig;
            return *this;
        }

        Cgi const &setLocation(const srv::location *location) {
            Cgi::_location = location;
            return *this;
        }

        bool setup() {
            if (!(_request && _response && _serverConfig && _location)) {
                return false;
            }
            std::pair<std::set<Field>::const_iterator, bool> ret;

            setenv("SERVER_SOFTWARE", "webserv/1.0");
            setenv("SERVER_NAME", _serverConfig->serverName.c_str());
            setenv("GATEWAY_INTERFACE", "CGI/1.1");
            setenv("SERVER_PROTOCOL", _request->getRequestLine().version);
            setenv("SERVER_PORT", ::util::size_tToString(_serverConfig->listen));
            setenv("REQUEST_METHOD", _request->getRequestLine().method);
            setenv("PATH_INFO", _request->getRequestLine().uri.path);
            setenv("PATH_TRANSLATED", _request->getRequestLine().uri.pathTranslated);
            setenv("SCRIPT_NAME", _request->getRequestLine().uri.path.substr(
                    _request->getRequestLine().uri.path.find_last_of('/') + 1));
            setenv("QUERY_STRING", _request->getRequestLine().uri.query);
            setenv("REDIRECT_STATUS", "200");
            if ((ret = _request->findField("Content-Type")).second) { setenv("CONTENT_TYPE", ret.first->value); }
            if ((ret = _request->findField("Content-Length")).second) { setenv("CONTENT_LENGTH", ret.first->value); }
//            setenv("HTTP_COOKIE", this->req.getHeader("Cookie").c_str(), 1);

            return true;
        }

        int exec() const {
            char const *env[_env.size() + 1];
            char const *argv[4] = {0};
            int pipeWrite[2];
            int pipeRead[2];
            pid_t pid;

            for (size_t i = 0; i < _env.size(); i++) {
                env[i] = _env[i].data();
            }
            env[_env.size()] = NULL;

            argv[0] = _request->getRequestLine().uri.pathTranslated.data();
            LOG_DEBUG("file(" << argv[0] << ')');
            argv[1] = NULL;

            if (pipe(pipeWrite) <= -1) {
                return FAILURE;
            }
            if (pipe(pipeRead) <= -1) {
                return FAILURE;
            }
            if ((pid = fork()) < 0) {
                return FAILURE;
            }
            if (!pid) {
                dup2(pipeWrite[0], STDIN_FILENO);
                close(pipeWrite[1]);
                dup2(pipeRead[1], STDOUT_FILENO);
                close(pipeRead[0]);
                execve(argv[0], (char *const *) argv, (char *const *) env);
                exit(1);
            } else {
                close(pipeRead[1]);
                close(pipeWrite[0]);
                for (size_t j = 0; j < _request->getBody().size(); ++j) {
                    j = write(pipeWrite[1], _request->getBody().data(), _request->getBody().size() - j);
                }
                close(pipeWrite[1]);

                char aux[__BUFFER_SIZE];
                bzero(aux, __BUFFER_SIZE);

                FILE *f = fdopen(pipeRead[0], "r");
                if (!f) { return FAILURE; }
                __gnu_cxx::stdio_filebuf<char> fileBuffer(f, std::ios::in);
                std::istream is(&fileBuffer);
                std::string line;
                LOG_DEBUG("Adding Fields...");
                while (std::getline(is, line) && line != "\r" && !line.empty()) {
                    _response->addField(line);
                }

                std::string body;
                while (std::getline(is, line)) {
                    body.append(line);
                }

                _response->setBodyFromString(body);
                close(pipeRead[0]);

                int status;
                wait(&status);
                if (status != SUCCESS) {
                    return FAILURE;
                }
                LOG_DEBUG("Status Child(" << status << ')');
                LOG("The CGI output" << _response->getBody());
                return 0;
            }
        }

    private:
        void setenv(std::string const &name, std::string const &value) {
            _env.push_back(name + '=' += value);
            LOG_DEBUG("setenv(" << name << '=' << value << ')');
        }

    private:
        Request const *_request;
        Response *_response;
        srv::ServerConfig const *_serverConfig;
        srv::location const *_location;
        std::vector<std::string> _env;
    };
} // http

#endif //INC_42_WEBSERV_CGI_HPP
