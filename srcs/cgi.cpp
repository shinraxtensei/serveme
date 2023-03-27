# include "../inc/cgi.hpp"
#include <cstdlib>
#include <stdlib.h>
#include <string>


Cgi::Cgi(){
    this->CONTENT_LENGTH = "";
    this->PATH_INFO = "";
    this->SCRIPT_FILENAME = "";
    this->CONTENT_TYPE = "";
    this->BODY = "";
    this->QUERY_MAP = std::map<std::string, std::string>();

    this->CompilerPathsByLanguage = std::map<std::string, std::string>{
        {"py", "/usr/bin/python"},
        {"php", "/usr/bin/php"}
    };    

}
// void Cgi::cgi_handler(){

    
//     std::string REQUEST_METHOD = "GET";
//     std::string CONTENT_LENGTH = "88";
//     std::string PATH_INFO = "/with/additional/path";
//     std::string SCRIPT_FILENAME = "Hello.py";
//     // std::string REDIRECT_STATUS = ;
//     std::string CONTENT_TYPE = "text/html";

//     // set env [ REQ ]

//     setenv("REQUEST_METHOD", REQUEST_METHOD.c_str(), 1);
//     setenv("CONTENT_LENGTH", CONTENT_LENGTH.c_str(), 1);
//     setenv("PATH_INFO", PATH_INFO.c_str(), 1);
//     setenv("SCRIPT_FILENAME", SCRIPT_FILENAME.c_str(), 1);
//     // setenv("REDIRECT_STATUS", REDIRECT_STATUS.c_str(), 1);
//     setenv("CONTENT_TYPE", CONTENT_TYPE.c_str(), 1);


//     char *env = getenv("PATH_INFO");
//     std::cout << env << std::endl;



// }

void Cgi::setEnv(std::string &Method){

    std::map<std::string, std::string>::iterator it;

    setenv("REQUEST_METHOD", Method.c_str(), 1);
    setenv("CONTENT_LENGTH", this->CONTENT_LENGTH.c_str(), 1);
    setenv("PATH_INFO", this->PATH_INFO.c_str(), 1);
    setenv("SCRIPT_FILENAME", this->SCRIPT_FILENAME.c_str(), 1);
    setenv("CONTENT_TYPE", this->CONTENT_TYPE.c_str(), 1);
    setenv("CONTENT_BODY", this->BODY.c_str(), 1);

    // this loop seting the env vars from the query map
    setenv("QUERY_STRING", this->QUERY_STRING.c_str(), 1);
    for (it = this->QUERY_MAP.begin(); it != this->QUERY_MAP.end(); ++it)
         setenv(it->first.c_str(), it->second.c_str(), 1);

}


// this function parse the url and return the path
std::string Cgi::parseUrl(std::string url){
    std::string parsedUrl = url;
    std::string::size_type pos = parsedUrl.find("?");
    if (pos != std::string::npos)
        parsedUrl = parsedUrl.substr(0, pos);
    return parsedUrl;
}

// this function parse the query and return a map of key value
std::map<std::string, std::string> Cgi::parseQuery(std::string query){


    std::map<std::string, std::string> querymap;

    std::string::size_type pos = query.find("?");
    if (pos != std::string::npos) {
        query = query.substr(pos + 1);
        this->QUERY_STRING = query;
        while (!query.empty()) {
            std::string::size_type pos2 = query.find("&");
            std::string keyvalue = query.substr(0, pos2);
            std::string::size_type pos3 = keyvalue.find("=");
            if (pos3 != std::string::npos) {
                querymap[keyvalue.substr(0, pos3)] = keyvalue.substr(pos3 + 1);
            }
            if (pos2 == std::string::npos) {
                break;
            }
            query = query.substr(pos2 + 1);
        }
    }
    return querymap;
}
// int main()
// {
//     std::string url;
//     Cgi cgi;
//     url = cgi.parseUrl("/with/additional/path?query=string");
//     std::cout << url << std::endl;
//     return 0;
// }