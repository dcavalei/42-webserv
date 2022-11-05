//
// Created by dcavalei on 07-08-2022.
//

#ifndef INC_42_WEBSERV_GETTERS_HPP
#define INC_42_WEBSERV_GETTERS_HPP

#include <string>

namespace http {
    namespace util {

        char const *getStatusCodePhrase(int statusCode) {
            switch (statusCode) {
                case 100: return "Continue";
                case 101: return "Switching Protocol";
                case 102: return "Processing";
                case 103: return "Early Hints";

                case 200: return "OK";
                case 201: return "Created";
                case 202: return "Accepted";
                case 203: return "Non-Authoritative Information";
                case 204: return "No Content";
                case 205: return "Reset Content";
                case 206: return "Partial Content";
                case 207: return "Multi-Status";
                case 208: return "Already Reported";
                case 226: return "IM Used";

                case 300: return "Multiple Choices";
                case 301: return "Moved Permanently";
                case 302: return "Found";
                case 303: return "See Other";
                case 304: return "Not Modified";
                case 305: return "Use Proxy";
                case 306: return "Switch Proxy";
                case 307: return "Temporary Redirect";
                case 308: return "Permanent redirect";

                case 400: return "Bad Request";
                case 401: return "Unauthorized";
                case 402: return "Payment Required";
                case 403: return "Forbidden";
                case 404: return "Not Found";
                case 405: return "Method Not Allowed";
                case 406: return "Not Acceptable";
                case 407: return "Proxy Authentication Required";
                case 408: return "Request Timeout";
                case 409: return "Conflict";
                case 410: return "Gone";
                case 411: return "Length Required";
                case 412: return "Precondition Failed";
                case 413: return "Payload Too Large";
                case 414: return "URI Too Long";
                case 415: return "Unsupported Media Type";
                case 416: return "Requested Range Not Satisfiable";
                case 417: return "Expectation Failed";
                case 418: return "418 I'm a teapot";
                case 421: return "Misdirected Request";
                case 422: return "Unprocessable Entity";
                case 423: return "Locked";
                case 424: return "Failed Dependency";
                case 425: return "Too Early";
                case 426: return "Upgrade Required";
                case 428: return "Precondition Required";
                case 429: return "Too Many Requests";
                case 431: return "Request Header Fields Too Large";
                case 451: return "Unavailable For Legal Reasons";

                case 500: return "Internal Server Error";
                case 501: return "Not Implemented";
                case 502: return "Bad Gateway";
                case 503: return "Service Unavailable";
                case 504: return "Gateway Timeout";
                case 505: return "HTTP Version Not Supported";
                case 506: return "Variant Also Negotiates";
                case 507: return "Insufficient Storage";
                case 508: return "Loop Detected";
                case 510: return "Not Extended";
                case 511 : return "Network Authentication Required";

//                case XXX: return "";

                default: return "Unknown";
            }
        }

        char const *getFileExtension(std::string const &path) {
            std::string extension = path.substr(path.find_last_of('.') + 1);

            if (extension == "html")
                return ("text/html");
            if (extension == "css")
                return ("text/css");
            if (extension == "js")
                return ("text/javascript");
            if (extension == "png")
                return ("image/png");
            if (extension == "jpg")
                return ("image/jpeg");
            if (extension == "jpeg")
                return ("image/jpeg");
            if (extension == "gif")
                return ("image/gif");
            if (extension == "svg")
                return ("image/svg+xml");
            if (extension == "mp4")
                return ("video/mp4");
            if (extension == "mp3")
                return ("audio/mp3");
            if (extension == "ogg")
                return ("audio/ogg");
            if (extension == "wav")
                return ("audio/wav");
            if (extension == "ico")
                return ("image/x-icon");
            if (extension == "ttf")
                return ("font/ttf");
            if (extension == "otf")
                return ("font/otf");
            if (extension == "woff")
                return ("font/woff");
            if (extension == "woff2")
                return ("font/woff2");
            if (extension == "pdf")
                return ("application/pdf");
            if (extension == "zip")
                return ("application/x-zip-compressed");
            if (extension == "rar")
                return ("application/x-rar-compressed");
            if (extension == "doc")
                return ("application/msword");
            if (extension == "docx")
                return ("application/vnd.openxmlformats-officedocument.wordprocessingml.document");
            if (extension == "xls")
                return ("application/vnd.ms-excel");
            if (extension == "xlsx")
                return ("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
            if (extension == "ppt")
                return ("application/vnd.ms-powerpoint");
            if (extension == "pptx")
                return ("application/vnd.openxmlformats-officedocument.presentationml.presentation");

            return ("text/plain");
        }

    } // util
} // http

#endif //INC_42_WEBSERV_GETTERS_HPP
