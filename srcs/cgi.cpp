# include "../inc/cgi.hpp"
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <istream>
#include <ostream>
#include <stdlib.h>
#include <string>
#include <sys/_types/_pid_t.h>
#include <sys/fcntl.h>
#include <unistd.h>


Cgi::Cgi(){
    this->CONTENT_LENGTH = "";
    this->PATH_INFO = "";
    this->SCRIPT_FILENAME = "";
    this->CONTENT_TYPE = "";
    this->BODY = "";
    // this->FULLBODY = make;
    this->QUERY_MAP = std::map<std::string, std::string>();

    this->CompilerPathsByLanguage = std::map<std::string, std::string>{
        {"py", "/usr/bin/python3"},
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

std::string Cgi::parseSurfix(std::string path_info){
    std::string surfix;
    std::size_t found = path_info.find_last_of(".");
    surfix = path_info.substr(found + 1);
    return surfix;
}
// int main()
// {
//     std::string url;
//     Cgi cgi;
//     url = cgi.parseUrl("/with/additional/path?query=string");
//     std::cout << url << std::endl;
//     return 0;
// }


void Client::cgi_handler(){

    std::cout << "******************** cgi_handler **********************" << std::endl;

    // there's a problem in the allowed methods from request
    std::vector<std::string> allowed_methods = {"GET", "POST",  "DELETE"};
    std::vector<std::string>::iterator it;

    this->cgi->CONTENT_LENGTH   = this->request->contentLength;
    this->cgi->PATH_INFO        = this->cgi->parseUrl(this->request->url);
    this->cgi->QUERY_MAP        = this->cgi->parseQuery(this->request->url);
    this->cgi->BODY             = this->request->bodyString;

    std::string surfix          = this->cgi->parseSurfix(this->cgi->PATH_INFO);
    this->cgi->CGI_PATH         = this->cgi->CompilerPathsByLanguage[surfix];
    if (this->cgi->CGI_PATH == "")
        std::cout << "CGI PATH IS EMPTY" << std::endl;
    else
        std::cout << "CGI PATH IS NOT EMPTY" << std::endl;
    it = find (allowed_methods.begin(), allowed_methods.end(), this->request->method);
    if (it == allowed_methods.end()){
        std::cout << "Return 405 ERROR" << std::endl;
        exit(100);
    }
    int piepfd[2];
    if (pipe(piepfd) == -1)
        std::cout << "Return 503 ERROR" << std::endl;
    
    /* *************************************************************************************************************** */
    if (this->request->method == "GET")
    {
        // here just teating the file output

        std::cout << "********* CGI FOR GET IS CALLEEDDDD **********" << std::endl;
        pid_t pid = fork();
        if (pid == -1)
            std::cout << "Return 503 ERROR" << std::endl;
        /* child process */
        if (pid == 0){
            dup2(int(piepfd[1]), 1);
            close(int(piepfd[0]));
            close(int(piepfd[1]));
            try {
                this->cgi->setEnv(this->request->method);
                extern char** environ;
                char** env = environ;
                this->cgi->PATH_INFO.erase(0, 1);
                char* arg[] = {strdup(this->cgi->CGI_PATH.c_str()), strdup(this->cgi->PATH_INFO.c_str()), NULL};
                char* path = strdup(this->cgi->CGI_PATH.c_str());
                
                if (execve(path, arg, env) == -1)
                    std::cout << "Return 503 ERROR" << std::endl;
                exit(0);
            } catch (...) {
                std::cout << "Error" << std::endl;
            }
            exit (1);
        }
    }
    /* *************************************************************************************************************** */
    else if (this->request->method == "POST")
    {

        std::cout << "********* CGI FOR POST IS CALLEEDDDD **********" << std::endl;
        pid_t pid = fork();
        if (pid == -1)
            std::cout << "Return 503 ERROR" << std::endl;
        /* child process */
        if (pid == 0){
            std::cout << "-------CHILD PROCESS BEGIN POST-------" << std::endl;
        
        
            std::string chnck = this->request->ParseBody();
            std::size_t found = chnck.find("\r\n\r\n");
            std::string bb = chnck.substr(found + 4);
            std::cout << "Body From Parser Direct :\n" << bb << std::endl;
            std::ofstream tmp("tmp.txt");
            tmp << bb;
            tmp.close();
            int fd = open("tmp.txt", O_RDONLY | O_CREAT | O_TRUNC, 0666);
            dup2(fd, 0);
            close(fd);
            dup2(int(piepfd[1]), 1);
            close(int(piepfd[0]));
            close(int(piepfd[1]));
            try {
                // this->cgi->setEnv(this->request->method);
                setenv("first_name", "Amine", 1);
                setenv("last_name", "READA", 1);
                // std::cout << "QUERY_STRING " << getenv("QUERY_STRING") << std::endl;
                extern char** environ;
                char** env = environ;
                this->cgi->PATH_INFO.erase(0, 1);
                char* arg[] = {strdup(this->cgi->CGI_PATH.c_str()), strdup(this->cgi->PATH_INFO.c_str()), NULL};
                char* path = strdup(this->cgi->CGI_PATH.c_str());
                if (execve(path, arg, env) == -1)
                    std::cout << "Return 503 ERROR" << std::endl;
            } catch (...) {
                std::cout << "Error" << std::endl;
            }
            std::cout << "-------CHILD PROCESS END POST-------" << std::endl;
            exit (1);
        }
        unlink("tmp.txt");
    }
    std::cout << "Parent Process" << std::endl;
    char buff;
    std::string body;
    waitpid(-1, 0, 0);
    // int flags = fcntl(piepfd[0], F_GETFL, 0);
    // fcntl(piepfd[0], F_SETFL, flags | O_NONBLOCK);
    close(int(piepfd[1]));
    while (read(piepfd[0], &buff, 1) > 0){
        // std::cout << "BODY: " << buff << std::endl;
        body.push_back(buff);
    }
    close(int(piepfd[0]));
    std::cout << "-----------------BODY FROM CGI-----------------\n" << body << std::endl;
    int bytes = send(this->request->client_fd, body.c_str(),  body.size(), 0);
    if (bytes == -1)
        std::cout << "Return 503 ERROR" << std::endl;
}