//
// Created by dcavalei on 25-07-2022.
//

#ifndef INC_42_WEBSERV_PARSER_HPP
#define INC_42_WEBSERV_PARSER_HPP

#include <fstream>
#include <cstring>
#include "util/util.hpp"
#include "srv/ServerConfig.hpp"
#include "util/macros.h"
#include "http/http.hpp"

namespace app {

  class Parser {
    public:
      explicit Parser(std::string const &path = "") : _path(path) {}

      Parser &setPath(std::string const &path) {
          _path = path;
          return *this;
      }

      std::set<srv::ServerConfig> parseFile(srv::ServerConfig &defaultServerConfig) {
          std::set<srv::ServerConfig> container;
          bool trigger = true;
          _file.open(_path.data(), std::ios_base::in);
          std::cout << &http::methods << std::endl;

          if (!_file) {
              throw std::invalid_argument(("open: " + std::string(std::strerror(errno))).c_str());
          }

          while (gnl()) {
              if (isComment()) continue;
              else if (isBlockOf("server")) {
                  LOG_DEBUG("Creating new serverConfig");
                  srv::ServerConfig target(createServer());

                  if (container.insert(target).second == false) {
                      LOG_WARNING("Failed to insert target serverConfig("
                                          << target.serverName << ':' << target.listen << ')');
                  } else {
                      LOG_DEBUG("Target serverConfig(" << target.serverName << ':' << target.listen
                                                       << ") inserted successfully");
                      if (trigger) {
                          defaultServerConfig = target;
                          trigger = false;
                      }
                  }
              }
          }
          _file.close();
          return container;
      }

    private:

      inline bool isBlockOf(const std::string &bn) const {
          return _line.find(bn) != std::string::npos;
      }

      inline bool isBlockEnd() const {
          return _line == "}";
      }

      inline bool isComment() const {
          return _line[0] == '#';
      }

      bool gnl() {
          if (_file.eof()) {
              return false;
          }
          std::getline(_file, _line);
          util::trim(_line);
          return true;
      }

      void folderCorrection(std::string &str) {
          if (str.at(0) != '/') {
              str = std::string("/") += str;
          }
          if (str.at(str.length() - 1) != '/') {
              str += '/';
          }
      }

      void serverNameHandler(srv::ServerConfig &server) {
          char buffer[128] = {0};
          std::sscanf(_line.c_str(), "server_name %[^;]", buffer);
          buffer[128 - 1] = 0;
          server.serverName = buffer;
          LOG_DEBUG("server_name " << server.serverName);
      }

      void listenHandler(srv::ServerConfig &server) {
          char buffer[32] = {0};
          std::sscanf(_line.c_str(), "listen %[^;]", buffer);
          buffer[32 - 1] = 0;
          server.listen = std::atoi(buffer);
          LOG_DEBUG("listen " << server.listen);
      }

      void clientMaxBodySizeHandler(srv::ServerConfig &server) {
          char buffer[32] = {0};
          std::sscanf(_line.c_str(), "client_max_body_size %[^;]", buffer);
          buffer[32 - 1] = 0;
          server.client_max_body_size = std::atoi(buffer) * __1MB;
          LOG_DEBUG("client_max_body_size " << server.client_max_body_size);
      }

      void autoindexHandler(srv::location &l) const {
          char buffer[4] = {0};
          std::sscanf(_line.c_str(), "autoindex %[^;]", buffer);
          buffer[4 - 1] = 0;
          if (std::strcmp(buffer, "on") == 0) {
              l.autoIndex = true;
          }
          LOG_DEBUG("autoindex " << std::boolalpha << l.autoIndex);
      }

      void redirectHandler(srv::location &l) const {
          char buffer[1024] = {0};
          int statusCode = 0;
          std::sscanf(_line.c_str(), "return %d %[^;]", &statusCode, buffer);
          buffer[1024 - 1] = 0;

          srv::redirect redirect;
          redirect.statusCode = statusCode;
          redirect.url = buffer;
          l.redirect = redirect;
          LOG_DEBUG("return " << l.redirect.value().statusCode << ' ' << l.redirect.value().url);
      }

      void cgiHandler(srv::location &l) const {
          char buffer[4] = {0};
          std::sscanf(_line.c_str(), "cgi %[^;]", buffer);
          buffer[4 - 1] = 0;
          if (std::strcmp(buffer, "on") == 0) {
              l.cgi = true;
          }
          LOG_DEBUG("cgi " << std::boolalpha << l.cgi);
      }

      void rootHandler(srv::location &l) {
          char buffer[1024] = {0};
          std::sscanf(_line.c_str(), "root %[^;]", buffer);
          buffer[1024 - 1] = 0;
          l.root = buffer;
          folderCorrection(l.root);
          LOG_DEBUG("root " << l.root);
      }

      void allowMethodsHandler(srv::location &l) {
          // TODO: parse this block
          std::stringstream ss(_line);
          std::set<std::string> am;
          ss >> _line;
          LOG_DEBUG("allowedMethods:");
          while (ss >> _line) {
              if (*(--_line.end()) == ';') {
                  _line.erase(--_line.end());
              }
              if (am.insert(_line).second) {
                  LOG_DEBUG(_line);
              }
          }
          l.allowedMethods = am;
      }

      void locationHandler(srv::ServerConfig &server) {
          srv::location l;
          // set location path
          {
              char buffer[128] = {0};
              std::sscanf(_line.c_str(), "location %[^ {]", buffer);
              buffer[128 - 1] = 0;
              l.path = buffer;
              folderCorrection(l.path);
              l.root = l.path;
              LOG_DEBUG("location " << l.path);
          }

          while (gnl() && !isBlockEnd()) {
              if (isComment())
                  continue;
              else if (isBlockOf("root"))
                  rootHandler(l);
              else if (isBlockOf("autoindex"))
                  autoindexHandler(l);
              else if (isBlockOf("return"))
                  redirectHandler(l);
              else if (isBlockOf("cgi"))
                  cgiHandler(l);
              else if (isBlockOf("allow_methods"))
                  allowMethodsHandler(l);
          }

          if (server.locations.insert(l).second) {
              LOG_DEBUG("location " << l.path << " inserted successfully");
          } else {
              LOG_WARNING("Failed to insert location " << l.path);
          }
      }

      void setDefaultBehaviour(srv::ServerConfig &serverConfig) {
          serverConfig.locations.insert(srv::location("/"));
      }

      srv::ServerConfig createServer() {
          srv::ServerConfig serverConfig;

          while (gnl() && !isBlockEnd()) {
              if (isComment())
                  continue;
              else if (isBlockOf("server_name"))
                  serverNameHandler(serverConfig);
              else if (isBlockOf("listen"))
                  listenHandler(serverConfig);
              else if (isBlockOf("client_max_body_size"))
                  clientMaxBodySizeHandler(serverConfig);
              else if (isBlockOf("location"))
                  locationHandler(serverConfig);
          }
          setDefaultBehaviour(serverConfig);
          LOG_DEBUG("serverConfig " << serverConfig.serverName << " finished");
          return serverConfig;
      }

      std::string _path;
      std::ifstream _file;
      std::string _line;
  };
} // app

#endif //INC_42_WEBSERV_PARSER_HPP
