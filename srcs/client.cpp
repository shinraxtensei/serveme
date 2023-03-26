#include "../inc/client.hpp"
#include <cstdlib>
#include <cstring>
#include <ios>
#include <iostream>
#include <ostream>
#include <string>
#include <sys/_types/_pid_t.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

Client::Client()
{
    this->addr = new sockaddr_in;

    this->request = new Request();
    this->request->core = this->core;
    this->request->client = this;

    this->response = new Response();
    this->response->http = this->core->get_http();
    this->response->client = this;

    this->pollfd_.fd = -1;
}

Client::~Client()
{
    // close(this->fd);
    // delete this->addr;
    // delete this->request;
    // delete this->response;
}

Client::Client(SocketWrapper &sock)
{
    this->cgi = new Cgi();
    this->request = new Request();
    this->request->core = this->core;
    // this->request->client = this;

    this->response = new Response();
    // this->response->http = this->core->get_http();
    // this->response->client = this;

    this->socket = &sock;
    addr = new sockaddr_in;
    fd = sock.accept(*addr);
    if (fd == -1)
    {
        throw std::runtime_error("Failed to accept connection");
    }
    this->request->client_fd = fd;
    this->response->client_fd = fd;
    // this->request->core = Servme::getCore();
    // this->request->client = &Servme::getCore()->map_clients[fd];

    fcntl(fd, F_SETFL, O_NONBLOCK);
    pollfd_.fd = fd;
    pollfd_.events = POLLIN;
}

std::string Request::checkType(std::string path)
{
    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos)
    {
        std::cout << "No extension" << std::endl;
        return "";
    }
    else
    {
        std::string extension = path.substr(dot + 1);
        std::map<std::string, std::string>::iterator iter;

        if (this->core->mimeTypes[extension] != "")
            return (this->core->mimeTypes[extension]);
        return ("");
    }
}

std::string checkForEnd(char c, int type)
{
    static int count = 0;
    if (type == 1)
    {
        if (c == '\r')
            count++;
        else if (c == '\n' && count == 2)
            count++;
        else if (c == '\r' && count == 3)
            count++;
        else if (c == '\n' && count == 4)
        {
            count = 0;
            return "\r\n\r\n";
        }
        else
            count = 0;
        return "";
    }
    else
    {
        if (c == '\r')
            count++;
        else if (c == '\n' && count == 1)
        {
            count = 0;
            return "\r\n";
        }
        else
            count = 0;
        return "";
    }
}

void Client::handleRequest()
{
    // if (this->request->state == START || this->request->state == Stat::FIRSTLINE || this->request->state == Stat::HEADERS)
    if (this->request->state & (Stat::START | Stat::FIRSTLINE | Stat::HEADERS))
    {
        static std::string line = "";
        char buffer[1];
        int ret;
        ret = recv(this->fd, buffer, 1, 0);
        if (ret == -1)
        {
            std::cerr << "Error: recv() failed" << std::endl;
            return;
        }
        else if (ret == 0)
        {
            std::cout << "disconnection" << std::endl;
            return;
        }
        line += buffer[0];
        this->request->buffer += buffer[0];
        if ((line.find("\r") != std::string::npos || line.find("\n") != std::string::npos || line.find("\r\n") != std::string::npos ) && this->request->state & Stat::START)
        {
            line = "";
            return;
        }
        if (this->request->state & Stat::START)
            this->request->state = Stat::FIRSTLINE;
        if (line.find("\r\n") != std::string::npos || line.find("\n") != std::string::npos)
        {
            if (line == "\r\n" || line == "\n")
                this->request->state = Stat::BODY;
            if (this->request->state & Stat::FIRSTLINE)
            {
                this->request->ParseFirstLine(line);
                this->request->state = Stat::HEADERS;
            }
            else if (this->request->state & Stat::HEADERS)
            {
                this->request->ParseHeaders(line);
            }
            line = "";
        }
        // if (this->request->buffer.find("\r\n\r\n") != std::string::npos)
        //     this->request->state = Stat::BODY;
    }

    else if (this->request->state & Stat::BODY)
    {


        if (this->request->bodyType == BodyType::CHUNKED)
        {
            if (this->request->bodyString.empty())
                this->request->state = Stat::CHUNKED_SIZE;
            this->request->ParseChunkedBody(); // ! : this function is not working ,still working on ti
        }
        else if (this->request->bodyType == BodyType::MULTIPART)
            this->request->ParseMultiPartBody(); // ! : this function is not working ,still working on ti
        else
            this->request->ParseBody();


        this->generateResponse();
        // writeResponse();
    }
}

void Client::cgi_handler(){

    std::vector<std::string>::iterator it;
    // there's a problem in the allowed methods from request
    std::vector<std::string> allowed_methods = {"GET", "POST",  "DELETE"};
    std::cout << "cgi_handler" << std::endl;

    this->cgi->CONTENT_LENGTH   = this->request->contentLength;
    this->cgi->PATH_INFO        = this->cgi->parseUrl(this->request->url);
    this->cgi->QUERY_MAP        = this->cgi->parseQuery(this->request->url);
    this->cgi->BODY             = this->request->bodyString;


    // }
    // for (std::vector<std::string>::iterator it = allowed_methods.begin(); it != allowed_methods.end(); ++it)
    //     std::cout << *it << std::endl;
    it = find (allowed_methods.begin(), allowed_methods.end(), this->request->method);
    if (it == allowed_methods.end()){
        std::cout << "Return 405 ERROR" << std::endl;
    //  generateResponse(); // wrong function jut to avoid error xD
    //  // return ERROR METHID NOT ALLOWD
        exit(100);
    }
    if (this->request->method == "GET")
    {
        std::cout << "********* CGI FOR GET IS CALLEEDDDD **********" << std::endl;
        int piepfd[2];
        this->cgi->QUERY_MAP = this->cgi->parseQuery(this->request->url);
        this->cgi->PATH_INFO = this->cgi->parseUrl(this->request->url);
        if (pipe(piepfd) == -1)
            std::cout << "Return 503 ERROR" << std::endl;
        pid_t pid = fork();
        if (pid == -1)
            std::cout << "Return 503 ERROR" << std::endl;
        if (pid == 0){
            std::cout << "CHILD PROCESS BEGIN" << std::endl;
            // dup2(int(piepfd[1]), 1);
            close(int(piepfd[0]));
            close(int(piepfd[1]));

            try {
                this->cgi->setEnv(this->request->method);
                std::cout << "QUERY_STRING " << getenv("QUERY_STRING") << std::endl;
                exit(0);
            } catch (...) {
                std::cout << "Error" << std::endl;
            }
            exit (1);
        }

        waitpid(-1, 0, 0);
        close(int(piepfd[0]));
        close(int(piepfd[1]));
    }

    // else if (this->request->method == "POST")
    // {
    //     // random string file_name;
    //     std::string file_name = "tmp_file";
    //     int piepfd[2];
    //     // this->cgi->querymap = this->cgi->parseQuery(this->request->bodyString);
    //     // this->cgi->PATH_INFO = this->cgi->parseUrl(this->request->bodyString);

 

    //     // std::cout << this->cgi->querymap << std::endl;
    //     std::cout << this->cgi->PATH_INFO << std::endl;
    //     std::cout << this->cgi->BODY << std::endl;
    //     if (pipe(piepfd) == -1)
    //         std::cout << "Return 503 ERROR" << std::endl;
    //     pid_t pid = fork();
    //     if (pid < 0)
    //         std::cout << "Return 503 ERROR" << std::endl;
    //     if (pid == 0){
    //         // handle signales
    //         setenv("REQUEST_METHOD", this->request->method.c_str(), 1);
    //         setenv("PATH_INFO", this->cgi->PATH_INFO.c_str(), 1);
    //         // setenv("CONTENT_LENGTH", this->request->contentLength.c_str(), 1);
    //         // setenv("CONTENT_TYPE", this->request->contentType.c_str(), 1);
    //         // setenv("SCRIPT_FILENAME", this->server->cgi_path.c_str(), 1);
    //         // setenv("QUERY_STRING", this->cgi->querymap.c_str(), 1);
    //         // // set env variables: REQ METH ; CONTENT LEN ; PATH INFO ;  SCRUPT FILE ; CONTENT TYPE; //////////////

    //         // open output file stream with the random file name
    //         std::ofstream tmpfile(file_name);
    //         // trim the body in the file
    //         // open fd on the random file with read permisson
    //         // dup the stander input to therandom file
    //         // run the script

    //         exit (1);
    //     }
    // }

}

// void	Client::generateResponse()
void Client::generateResponse()
{
    std::cout << "generate response" << std::endl;
    // this->selectServer();
    // this->server->server_name = "localhost";
    // std::cout << "selected server" << this->server->server_name << std::endl;
    // this->response->checkCgi();
    cgi_handler();
    // if (this->cgiFlag == 1)
    // {
    //     // handlecgi;
    // }
    // else
    // {
    //     this->response->checkAllowedMethods();
    //     // this->path = this->location->root + this->request->url;
    //     // std::cout << "path: " << this->path << std::endl;
    //     this->response->matchLocation();
    //     // handle manual;
    // }
    // std::cout << "cgi ? " << this->cgiFlag << std::endl;
}

void Client::selectServer()
{
    std::vector<Server>::iterator it;
    std::vector<Server> candidates;

    it = this->core->get_http()->servers.begin();
    for (it = this->core->get_http()->servers.begin(); it != this->core->get_http()->servers.end(); it++)
    {
        if (it->ipPort.second == this->socket->get_listenPair().second)
            candidates.push_back(*it);
    }
    if (candidates.size() == 0)
        throw std::runtime_error("Error: No server found for this request.");
    else
    {
        for (it = candidates.begin(); it != candidates.end(); it++)
        {
            if (it->server_name == this->request->host)
            {
                // this->server = &(*it);
                this->server = new Server(*it);
                return;
            }
        }
        this->server = new Server(candidates[0]);
    }
}
