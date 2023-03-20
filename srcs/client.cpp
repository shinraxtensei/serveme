#include "../inc/client.hpp"

Client::Client()
{
    this->addr = new sockaddr_in;

    this->request = new Request();
    this->request->core = this->core;
    this->request->client = this;

//   this->response = new Response();
//   this->response->core = this->core;
//   this->response->client = this;
   
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
    this->request = new Request();
    this->request->core = this->core;
    this->request->client = this;

//   this->response = new Response();
//   this->response->core = this->core;
//   this->response->client = this;


    this->socket = &sock;
    addr = new sockaddr_in;
    fd = sock.accept(*addr);
    if (fd == -1)
    {
        throw std::runtime_error("Failed to accept connection");
    }
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

std::string checkForEnd(char c , int type)
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



    if (this->request->state == FIRSTLINE || this->request->state == HEADERS)
    {
        static std::string line = "";
        char buffer[1];
        int ret;
        ret = recv(this->fd, buffer, 1, 0);
        if (ret == -1)
        {
            // handle error
            std::cerr << "Error: recv() failed" << std::endl;

            return;
        }
        else if (ret == 0)
        {
            // disconnection
            std::cout << "disconnection" << std::endl;
            return;
        }

        line += buffer[0];
        this->request->buffer += buffer[0];

        if (line.find("\r\n") != std::string::npos || line.find("\n") != std::string::npos)
        {
            if (this->request->state == FIRSTLINE)
            {
                this->request->ParseFirstLine(line);
                this->request->state = HEADERS;
            }
            else if (this->request->state == HEADERS)
            {
                this->request->ParseHeaders(line);
                if (line == "\r\n" || line == "\n")
                    this->request->state = BODY;
            }
            line = "";
        }

        if (this->request->buffer.find("\r\n\r\n") != std::string::npos)
            this->request->state = BODY;
    }
    else if (this->request->state == BODY)
    {
        char buffer[1024];
        int ret;
        ret = recv(this->fd, buffer, 1024, 0);
        if (ret == -1)
        {
            // handle error
            std::cerr << "Error: recv() failed" << std::endl;
            return;
        }
        else if (ret == 0)
        {
            // disconnection
            std::cout << "disconnection" << std::endl;

            return;
        }
        this->request->bodyString += std::string(buffer, ret);
        this->request->buffer += std::string(buffer, ret);
        this->request->ParseBody();
        // generateResponse();
        // writeResponse();
    }
}

void Client::cgi_handler(){

    
    this->cgi->REQUEST_METHOD   = this->request->method;
    this->cgi->CONTENT_LENGTH   = this->request->contentLength;
    this->cgi->PATH_INFO        = this->cgi->parseUrl(this->request->url);
    this->cgi->querymap         = this->cgi->parseQuery(this->request->url);
    this->cgi->BODY             = this->request->bodyString;

    std::cout << "REQUEST_METHOD: " << this->cgi->REQUEST_METHOD << std::endl;
    std::cout << "CONTENT_LENGTH: " << this->cgi->CONTENT_LENGTH << std::endl;
    std::cout << "PATH_INFO: " << this->cgi->PATH_INFO << std::endl;
    std::cout << "BODY: " << this->cgi->BODY << std::endl;
    for (std::map<std::string, std::string>::iterator it = this->cgi->querymap.begin(); it != this->cgi->querymap.end(); ++it)
        std::cout << it->first << " => " << it->second << '\n';

    // set env [ REQ ]

    // setenv("REQUEST_METHOD", REQUEST_METHOD.c_str(), 1);
    // setenv("CONTENT_LENGTH", CONTENT_LENGTH.c_str(), 1);
    // setenv("PATH_INFO", PATH_INFO.c_str(), 1);
    // setenv("SCRIPT_FILENAME", SCRIPT_FILENAME.c_str(), 1);
    // // setenv("REDIRECT_STATUS", REDIRECT_STATUS.c_str(), 1);
    // setenv("CONTENT_TYPE", CONTENT_TYPE.c_str(), 1);


    char *env = getenv("PATH_INFO");
    std::cout << env << std::endl;

}