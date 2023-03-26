#pragma once

#include "servme.hpp"
#include <map>
#include <string>

class Cgi{

    public:
        Cgi();
        // Cgi(const Cgi &other);
        // Cgi &operator=(const Cgi &other);
        // ~Cgi();
        std::string REQUEST_METHOD;
        std::string CONTENT_LENGTH;
        std::string PATH_INFO;
        std::string SCRIPT_FILENAME;
        std::string CONTENT_TYPE;
        std::string BODY;
        std::map<std::string, std::string> querymap;

        std::string parseUrl(std::string url);
        std::map<std::string, std::string> parseQuery(std::string query);
    // public:
    //     void cgi_handler();
};