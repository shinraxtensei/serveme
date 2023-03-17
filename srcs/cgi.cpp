# include "../inc/cgi.hpp"
#include <cstdlib>
#include <stdlib.h>




void Cgi::cgi_handler(){

    std::string REQUEST_METHOD = "GET";
    std::string CONTENT_LENGTH = "88";
    std::string PATH_INFO = "/with/additional/path";
    std::string SCRIPT_FILENAME = "Hello.py";
    // std::string REDIRECT_STATUS = ;
    std::string CONTENT_TYPE = "text/html";

    // set env [ REQ ]

    setenv("REQUEST_METHOD", REQUEST_METHOD.c_str(), 1);
    setenv("CONTENT_LENGTH", CONTENT_LENGTH.c_str(), 1);
    setenv("PATH_INFO", PATH_INFO.c_str(), 1);
    setenv("SCRIPT_FILENAME", SCRIPT_FILENAME.c_str(), 1);
    // setenv("REDIRECT_STATUS", REDIRECT_STATUS.c_str(), 1);
    setenv("CONTENT_TYPE", CONTENT_TYPE.c_str(), 1);


    char *env = getenv("PATH_INFO");
    std::cout << env << std::endl;



}

int main()
{
    Cgi cgi;
    cgi.cgi_handler();
    return 0;
}