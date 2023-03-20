#pragma once

#include "servme.hpp"
#include <string>

class Cgi{

    public:
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