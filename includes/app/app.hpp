#include <csignal>
//
// Created by dcavalei on 25-07-2022.
//

#ifndef INC_42_WEBSERV_APP_HPP
#define INC_42_WEBSERV_APP_HPP

#include "app/Parser.hpp"
#include "http/Cgi.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include "http/http.hpp"
#include "http/util/getters.hpp"
#include "net/Epoll.hpp"
#include "srv/Server.hpp"
#include "util/macros.h"
#include "util/util.hpp"
#include <cstdio>
#include <dirent.h>
#include <iostream>
#include <list>

namespace app {

    class app {
    public:
        app(int argc, char *argv[], std::string const &configPath, Parser *parser, net::Epoll *epoll)
                : _argc(argc),
                  _argv(argv),
                  _configPath(configPath),
                  _parser(parser),
                  _epoll(epoll) {
            LOG_DEBUG("Dependencies injected");
        }

        ~app() {
        }

        bool load() {
            std::set<srv::ServerConfig> srvConfSet;

            LOG_DEBUG("____________________INIT PARSING____________________");
            if (_argc > 2) {
                LOG_ERROR("Usage: " << _argv[0] << " /path/to/config");
                return false;
            } else {
                if (_argc == 2) {
                    LOG_DEBUG("Using CLI arguments path...");
                    _configPath = _argv[1];
                } else {
                    LOG_DEBUG("Using default config path...");
                }
                LOG_DEBUG("Config path: " << _configPath);
            }

            _parser->setPath(_configPath);
            srvConfSet = _parser->parseFile(_defaultServerConfig);
            LOG_DEBUG("____________________END PARSING____________________");

            setup(srvConfSet);

            LOG_DEBUG("____________________PRINT CONTAINERS____________________");

            LOG_DEBUG("PortSocketMap content:");
            for (std::map<in_port_t, net::TCPSocket>::iterator it = _portSocketMap.begin();
                 it != _portSocketMap.end(); ++it) {
                LOG_DEBUG('[' << it->first << "][" << it->second << ']');
            }

            LOG_DEBUG("ServerMap content:");
            for (std::map<std::string, srv::Server>::iterator it = _serverMap.begin();
                 it != _serverMap.end(); ++it) {
                LOG_DEBUG('[' << it->first << "][" << it->second.getServerConfig().serverName << ']');
            }

            return true;
        }

        void start() {
            _epoll->create();
            addServersToEpoll();

            // main loop
            while (42) {
                LOG_DEBUG("Epoll waiting...");
                std::pair<int, struct epoll_event *> pair = _epoll->wait(-1);
                LOG_DEBUG("Epoll woke up...");

                int nfds = pair.first;
                struct epoll_event *ev = pair.second;

                for (int i = 0; i < nfds; ++i) {
                    int eventFd = ev[i].data.fd;
                    uint32_t event = ev[i].events;

                    if (event & EPOLLRDHUP) {
                        LOG_DEBUG("Client closed connection...");
                        remove(eventFd);
                        continue;
                    }

                    net::TCPSocket socket;
                    if ((socket = isServerFd(eventFd)) != -1) {
                        LOG_DEBUG("New connection...");

                        int conn_sock = socket.accept();
                        if (fcntl(conn_sock, F_SETFL, O_NONBLOCK) == -1) {
                            LOG_ERROR("Error setting socket to non blocking");
                        }
                        _epoll->add(conn_sock, EPOLLIN | EPOLLET | EPOLLRDHUP);
                    } else {
                        LOG_DEBUG("Handling Request...");
                        http::Request request;
                        http::Response response;

                        requestHandler(request, response, eventFd);
                        setupConnection(request, response);
                        setupCookies(request, response);
                        response.sendResponse(eventFd);
                    }
                }
            }
        }

    private:
        static void setErrorPage(http::Response &response, int statusCode) {
            std::stringstream ss;
            ss << '/' << statusCode << ".html";

            response.setupStatusLine(statusCode)
                    .setBodyFromFile(STATUS_PAGE(ss.str()))
                    .addField("Content-Type", http::util::getFileExtension(".html"))
                    .addField("Content-Length", util::size_tToString(response.getBody().size()));
        }

        static inline void setPage(http::Response &response, int statusCode) {
            setErrorPage(response, statusCode);
        }

        void requestHandler(http::Request &request, http::Response &response, int eventFd) {
            int ret;
            if ((ret = request.parse(eventFd)) != SUCCESS) {
                setErrorPage(response, ret);
                return;
            }

            srv::ServerConfig const &tgtSrvConf = getTargetServerConfig(request);
            srv::location const &tgtLoc = getTgtLocation(tgtSrvConf,
                                                         request.getRequestLine().uri.path);

            if ((ret = checkBodySize(request, tgtSrvConf)) != SUCCESS) {
                setErrorPage(response, ret);
                return;
            }

            setupRoot(request, tgtLoc);

            LOG_DEBUG("Target ServerConfig(" << tgtSrvConf.serverName << ':' << tgtSrvConf.listen << ')');
            LOG_DEBUG("Target location(" << tgtLoc.path << ')');

            if (!isMethodAllowed(request, tgtLoc)) {
                LOG_WARNING("Method " << request.getRequestLine().method << " not allowed");
                setErrorPage(response, 405);
            } else if (tgtLoc.redirect.has_value()) {
                LOG_DEBUG("Redirected...");
                response.setupStatusLine(tgtLoc.redirect.value().statusCode)
                        .addField("location", tgtLoc.redirect.value().url)
                        .addField("Connection", "close");
            } else if (tgtLoc.cgi) {
                LOG_DEBUG("CGI...");
                http::Cgi c(&request, &response, &tgtSrvConf, &tgtLoc);
                c.setup();
                if (c.exec() == SUCCESS) {
                    response.setupStatusLine(200);
                } else {
                    setErrorPage(response, 500);
                }
            } else {
                LOG_DEBUG("Changing RequestLine.Uri.PathTranslated("
                                  << request.getRequestLine().uri.pathTranslated << ")");
                if (*request.getRequestLine().uri.pathTranslated.rbegin() == '/') {
                    LOG_DEBUG("Appending index.html");
                    request.changeUriPathTranslated(
                            request.getRequestLine().uri.pathTranslated + "index.html");
                }

                if (request.getRequestLine().method == "GET") {
                    LOG_DEBUG("GET method...");
                    if (!fileExist(request.getRequestLine().uri.pathTranslated)) {
                        if (tgtLoc.autoIndex) {
                            LOG_DEBUG("autoindex...");
                            std::stringstream auto_index;
                            std::string dirPath(request.getRequestLine().uri.path);
                            std::string dirPathTranslated(request.getRequestLine().uri.pathTranslated);
                            std::string::size_type pos;

                            if (*dirPath.rbegin() != '/' && (pos = dirPath.find_last_of('/')) != std::string::npos) {
                                dirPath.erase(pos);
                            }
                            if (*dirPathTranslated.rbegin() != '/' &&
                                (pos = dirPathTranslated.find_last_of('/')) != std::string::npos) {
                                dirPathTranslated.erase(pos);
                            }
                            LOG_DEBUG("dirPath(" << dirPath << ')');
                            LOG_DEBUG("dirPathTranslated(" << dirPathTranslated << ')');

                            auto_index << "<!DOCTYPE html>\n"
                                          "<html lang=\"en\">\n"
                                          "<head>\n"
                                          "<meta charset=\"UTF-8\">\n"
                                          "<title>autoindex</title>\n"
                                          "</head>\n"
                                          "<body>\n"
                                          "<h1>Autoindex of "
                                       << dirPath << "</h1>\n";

                            DIR *dir;
                            struct dirent *diread;
                            std::map<std::string, int> map;

                            if ((dir = opendir(dirPathTranslated.data())) != 0) {
                                while ((diread = readdir(dir)) != 0) {
                                    map[diread->d_name] = diread->d_type;
                                }
                                closedir(dir);
                            } else {
                                perror("opendir");
                            }
                            for (std::map<std::string, int>::iterator it = map.begin(); it != map.end(); it++) {
                                auto_index << "<p><a href=\"" << it->first << (it->second == DT_DIR ? "/" : "")
                                           << "\"</a>" << it->first << (it->second == DT_DIR ? "/" : "")
                                           << "</p>\n";// Adding the '/' or not.
                            }

                            auto_index << "</body>\n"
                                          "</html>";
                            response.setupStatusLine(200)
                                    .setBodyFromString(auto_index.str())
                                    .addField("Content-Type", http::util::getFileExtension(".html"))
                                    .addField("Content-Length", util::size_tToString(response.getBody().size()));
                        } else {
                            setErrorPage(response, 404);
                        }
                    } else {
                        response.setupStatusLine(200)
                                .setBodyFromFile(request.getRequestLine().uri.pathTranslated)
                                .addField("Content-Type",
                                          http::util::getFileExtension(request.getRequestLine().uri.pathTranslated))
                                .addField("Content-Length",
                                          util::size_tToString(response.getBody().size()));
                    }
                } else if (request.getRequestLine().method == "POST") {
                    LOG_DEBUG("POST method...");

                    std::ofstream f(request.getRequestLine().uri.pathTranslated.data());
                    if (f.is_open()) {
                        f << request.getBody();
                        f.close();
                        if (f.bad()) {
                            setErrorPage(response, 500);
                        }
                        setPage(response, 200);
                    } else {
                        LOG_DEBUG("asdasd");
                        setErrorPage(response, 500);
                    }
                } else if (request.getRequestLine().method == "DELETE") {
                    LOG_DEBUG("DELETE method...");
                    if (std::remove(request.getRequestLine().uri.pathTranslated.data())) {
                        LOG_WARNING("File not Deleted...");
                        setErrorPage(response, 404);
                    } else {
                        LOG_DEBUG("File Deleted...");
                        setPage(response, 200);
                    }
                }
            }
        }

        static int
        checkBodySize(const http::Request &request, const srv::ServerConfig &tgtSrvConf) {
            std::pair<std::set<http::Field>::const_iterator, bool> pair = request.findField("Content-Length");
            if (pair.second &&
                (static_cast<size_t>(atoi(pair.first->value.data())) > tgtSrvConf.client_max_body_size ||
                 request.getBody().size() > tgtSrvConf.client_max_body_size)) {
                LOG_DEBUG("client_max_body_size: " << tgtSrvConf.client_max_body_size
                                                   << "\nContent-Length: " << pair.first->value.data()
                                                   << "\nBody-Size: " << request.getBody().size());
                return 413;
            }
            LOG_DEBUG("Request Body fits :)");
            return SUCCESS;
        }

        static void setupRoot(http::Request &request, const srv::location &targetLocation) {
            std::string path = request.getRequestLine().uri.path;
            size_t pos = path.find(targetLocation.path);
            size_t len = targetLocation.path.size();

            path.replace(pos, len, targetLocation.root);
            request.changeUriPathTranslated(WWW_PAGE(path));
            LOG_DEBUG("Request.RequestLine.uri.pathTranslated => " << request.getRequestLine().uri.pathTranslated);
        }

        static inline bool fileExist(const std::string &name) {
            std::ifstream f(name.data());
            return f.good();
        }

        static void setupConnection(http::Request const &request, http::Response &response) {
            std::pair<std::set<http::Field>::const_iterator, bool> ret = request.findField("Connection");
            if (ret.second && ret.first->value == "Close") {
                response.addField("Connection", "close");
            }
        }

        static void setupCookies(http::Request const &request, http::Response &response) {
            std::pair<std::set<http::Field>::const_iterator, bool> ret = request.findField("Cookie");
            if (!ret.second) {
                LOG_DEBUG("No Cookie");
                std::stringstream ss;
                ss << util::randomNumber();
                response.addField("Set-Cookie", "id=" + ss.str());
            } else {
                LOG_DEBUG("Cookie found hue");
            }
        }

        static inline bool isMethodAllowed(http::Request const &request, srv::location const &targetLocation) {
            return (targetLocation.allowedMethods.find(request.getRequestLine().method) !=
                    targetLocation.allowedMethods.end());
        }

        void remove(int fd) {
            LOG_DEBUG("Removing client...");
            _epoll->remove(fd);
            close(fd);
        }

        void addServersToEpoll() {
            for (std::map<in_port_t, net::TCPSocket>::iterator it = _portSocketMap.begin();
                 it != _portSocketMap.end(); ++it) {
                if (_epoll->add(it->second, EPOLLIN) == -1) {
                    LOG_ERROR("Failed to add fd(" << it->second << ")");
                }
            }
        }

        srv::location const &getTgtLocation(srv::ServerConfig const &serverConfig, std::string const &path) const {
            std::string::size_type pos, last_pos;
            std::string folderPath(path);
            last_pos = std::string::npos;

            while ((pos = folderPath.find_last_of('/', last_pos)) != std::string::npos) {
                LOG_DEBUG("--folderPath before (" << folderPath << ")");
                folderPath.erase(pos + 1);
                LOG_DEBUG("folderPath after (" << folderPath << ")--");
                std::set<srv::location>::iterator it = serverConfig.locations.find(
                        srv::location(folderPath.data()));
                if (it != serverConfig.locations.end()) {
                    LOG_DEBUG("Path(" << path << ") belongs to location("
                                      << it->path
                                      << ") with Root(" << it->root << ")");
                    return *it;
                }
                last_pos = pos - 1;
            }
            return *_defaultServerConfig.locations.begin();
        }

        srv::ServerConfig const &getTargetServerConfig(http::Request const &request) const {
            std::set<http::Field>::const_iterator it = request.getFields().find(http::Field("Host"));
            if (it == request.getFields().end() || _serverMap.find(it->value) ==
                                                   _serverMap.end()) {// if Host not specified or Server not found
                LOG_DEBUG("Using default ServerConfig(" << _defaultServerConfig.serverName << ':'
                                                        << _defaultServerConfig.listen << ")");
                return _defaultServerConfig;
            } else {// else Host found
                srv::ServerConfig const &serverConfig = _serverMap.find(it->value)->second.getServerConfig();
                LOG_DEBUG("Using ServerConfig(" << serverConfig.serverName << ':'
                                                << serverConfig.listen << ")");
                return serverConfig;
            }
        }

        static std::string server_port(std::string const &name, int port) {
            std::stringstream ss;
            ss << name << ':' << port;
            return ss.str();
        }

        net::TCPSocket isServerFd(int fd) {
            for (std::map<in_port_t, net::TCPSocket>::iterator it = _portSocketMap.begin();
                 it != _portSocketMap.end(); ++it) {
                if (it->second.getFd() == fd) {
                    return it->second;
                }
            }

            // default with socket(-1);
            return net::TCPSocket();
        }

        void setup(const std::set<srv::ServerConfig> &srvConfSet) {
            LOG_DEBUG("____________________INIT [PORT-SOCKET-SERVER] BINDING____________________");
            for (std::set<srv::ServerConfig>::iterator it = srvConfSet.begin(); it != srvConfSet.end(); ++it) {
                std::map<in_port_t, net::TCPSocket>::iterator target;
                srv::Server server(*it);
                std::string name(server_port(server.getServerConfig().serverName, server.getServerConfig().listen));

                LOG_DEBUG("Server(" << name << ") created");

                target = _portSocketMap.find(server.getServerConfig().listen);
                if (target != _portSocketMap.end()) {
                    LOG_WARNING("Port already in use...");
                    LOG_DEBUG("Set Socket(" << target->second << ") to Server(" << name << ')');
                    server.setSocket(target->second);
                } else {
                    LOG_DEBUG("Port available...");
                    server.initSocket();
                    LOG_DEBUG("Socket(" << server.getSocket() << ") initialized");
                    _portSocketMap.insert(
                            std::make_pair(server.getServerConfig().listen, server.getSocket()));
                    LOG_DEBUG("Inserted Port(" << server.getServerConfig().listen << ") with Socket("
                                               << server.getSocket() << ") to portSocketMap");
                }
                _serverMap.insert(std::make_pair(name, server));
                LOG_DEBUG("Server(" << name << ") inserted to serverSet");
            }
            LOG_DEBUG("____________________END [PORT-SOCKET-SERVER] BINDING____________________");
        }

    public:
    private:
        int _argc;
        char const *const *_argv;
        std::string _configPath;
        Parser *_parser;
        net::Epoll *_epoll;
        // internal data members;
        std::map<std::string, srv::Server> _serverMap;
        std::map<in_port_t, net::TCPSocket> _portSocketMap;
        srv::ServerConfig _defaultServerConfig;
        //        static volatile bool on;
    };
}// namespace app

#endif//INC_42_WEBSERV_APP_HPP
