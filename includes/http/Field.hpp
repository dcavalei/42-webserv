//
// Created by dcavalei on 28-07-2022.
//

#ifndef INC_42_WEBSERV_FIELD_HPP
#define INC_42_WEBSERV_FIELD_HPP

namespace http {
    struct Field {
        Field(char const * const name = "", char const * const value = "")
                : name(name), value(value) {}

        bool operator<(Field const &other) const {
            return name < other.name;
        }

        std::string fieldToHttpLine() const {
            std::string str;
            str.reserve(name.length() + 2 + value.length() + 2);
            str += name;
            str += ": ";
            str += value;
            str += "\r\n";
            return str;
        }

        std::string name;
        std::string value;
    };

} // http

#endif //INC_42_WEBSERV_FIELD_HPP
