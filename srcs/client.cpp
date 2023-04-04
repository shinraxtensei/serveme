#include "../inc/client.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <ostream>
#include <string>
#include <sys/_types/_pid_t.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
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
    this->server = nullptr;
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
            this->pollfd_.fd = -1;
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
            {
                if (this->request->method == "GET")
                    this->response->GENERATE_RES = true;
                this->request->state = Stat::BODY;
            }
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
        if (this->request->buffer.find("\r\n\r\n") != std::string::npos)
            this->request->state = Stat::BODY;
    }

    else if (this->request->state & Stat::BODY)
    {

        
        static int flag = 0;

        this->request->ParseBody();
        if (this->request->bodyType == BodyType::CHUNKED)
        {
            if (flag == 0)
            {
                flag = 1;
                std::cout << BLUE <<"this is the start of chunked body" << RESET << std::endl;
                this->request->state = Stat::CHUNKED_START;
            }
            this->request->ParseChunkedBody(); // ! : this function is not working ,still working on ti
        }

        else if (this->request->bodyType == BodyType::MULTIPART)
        {
            if (flag == 0)
            {
                flag = 1;
                std::cout << BLUE <<"this is the start of multipart body" << RESET << std::endl;
                this->request->state = Stat::MULTI_PART_START;
            }
            this->request->ParseMultiPartBody(); // ! : this function is not working ,still working on ti
        }
        // else


        // writeResponse();
    	// this->generateResponse();
    }
    else if (this->request->state == Stat::END)
    {
        

        this->pollfd_.events &= ~POLLOUT;
    }
    if (this->response->GENERATE_RES && this->request->method == "GET")
    {
        std::cout << "generate response" << std::endl;
        this->generateResponse();
        // this->response->GENERATE_RES = false;
    }
    else if ((this->request->method == "POST" || this->request->method == "DELETE") && this->response->GENERATE_RES)
        this->generateResponse();

}

void Client::cgi_handler(){

    std::cout << "******************** cgi_handler **********************" << std::endl;

    // there's a problem in the allowed methods from request
    std::vector<std::string> allowed_methods = {"GET", "POST",  "DELETE"};
    std::vector<std::string>::iterator it;

    this->cgi->CONTENT_LENGTH   = this->request->contentLength;
    this->cgi->PATH_INFO        = this->cgi->parseUrl(this->request->url);
    this->cgi->QUERY_MAP        = this->cgi->parseQuery(this->request->url);
    this->cgi->BODY             = this->request->bodyString;

    std::string surfix = this->cgi->parseSurfix(this->cgi->PATH_INFO);
    this->cgi->CGI_PATH         = this->cgi->CompilerPathsByLanguage[surfix];
    if (this->cgi->CGI_PATH == "")
        std::cout << "CGI PATH IS EMPTY" << std::endl;
    else
        std::cout << "CGI PATH IS NOT EMPTY" << std::endl;

    std::cout << "[PY : " << this->cgi->CompilerPathsByLanguage["py"] << "]" << std::endl;
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
    int piepfd[2];
    if (pipe(piepfd) == -1)
        std::cout << "Return 503 ERROR" << std::endl;
    
    std::cout << "From BodyString: " << this->request->bodyString  << std::endl;
    if (this->request->method == "GET")
    {
        // here just teating the file output

        std::cout << "********* CGI FOR GET IS CALLEEDDDD **********" << std::endl;
        std::cout << "URL: " << this->request->url << std::endl;
        std::cout << "PATH INFO: " << this->cgi->PATH_INFO << std::endl;
        pid_t pid = fork();
        if (pid == -1)
            std::cout << "Return 503 ERROR" << std::endl;
        /* child process */
        if (pid == 0){
            std::cout << "CHILD PROCESS BEGIN" << std::endl;
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
    else if (this->request->method == "POST")
    {
        // random string file_name;
        std::cout << "********* CGI FOR GET IS CALLEEDDDD **********" << std::endl;
        std::cout << "URL: " << this->request->url << std::endl;
        std::cout << "PATH INFO: " << this->cgi->PATH_INFO << std::endl;
        pid_t pid = fork();
        if (pid == -1)
            std::cout << "Return 503 ERROR" << std::endl;
        /* child process */
        if (pid == 0){
            std::ofstream tmp("tmpfile");

            this->request->bodyString = this->request->bodyString.erase(this->request->bodyString.find_last_of("\r\n\r\n") + 1); 
            tmp << this->request->bodyString;
            std::cout << "CHILD PROCESS BEGIN" << std::endl;
            // dup2(int(piepfd[1]), 1);
            int fd = open("tmpfile", O_RDONLY);
            dup2(fd, 0);
            // std::cout << toDelFD << std::endl;
            close(int(piepfd[0]));
            close(int(piepfd[1]));
            close(fd);

            try {
                this->cgi->setEnv(this->request->method);
                // std::cout << "QUERY_STRING " << getenv("QUERY_STRING") << std::endl;
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
    // if (this->request->state == BODY){

    //     this->pollfd_.events |= POLLOUT;
    //     std::string response = "HTTP/1.1 200 OK\n"
    //                       "Date: Fri, 25 Mar 2023 09:30:00 GMT\n"
    //                       "Server: Apache/2.4.48 (Unix) OpenSSL/1.1.1k\n"
    //                       "Last-Modified: Thu, 24 Mar 2023 16:40:00 GMT\n"
    //                       "ETag: \"10a0-5a88d50f8a940\"\n"
    //                       "Accept-Ranges: bytes\n"
    //                       "Content-Length: 2576\n"
    //                       "Content-Type: text/html\n"
    //                       "\n"
    //                       "<!DOCTYPE html>\n"
    //                       "<html>\n"
    //                       "<head>\n"
    //                       "  <title>Example</title>\n"
    //                       "</head>\n"
    //                       "<body>\n"
    //                       "  <h1>Hello, World!</h1>\n"
    //                       "  <p>This is an example response.</p>\n"
    //                       "</body>\n"
    //                       "</html>\n";
    //     // this->request->client_fd;
    //     int bytes = send(this->request->client_fd, response.c_str(), response.size(), 0);
    //     this->pollfd_.events &= ~POLLOUT;

    //     if (bytes < 0)
    //         std::cout << "<h1>Error</h1>" << std::endl;
        // close(this->request->client_fd);
        // exit(1);
    //     // std::cout << "BODY: " << body << std::endl;
    // }


    char buff;
    std::string body;
    waitpid(-1, 0, 0);
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
    // if (this->response->GENERATE_RES)
    // {
    //     std::cout << "generate response" << std::endl;
    //     // this->generateResponse();
    //     // this->response->GENERATE_RES = false;
    // }
}

void Client::generateResponse()
{
	this->response->client = &Servme::getCore()->map_clients[this->response->client_fd];
	// this->response->checkAllowedMethods(); // error here aborted
	this->response->checkCgi();
	if (this->cgiFlag == 1)
        cgi_handler();
	else
		this->response->handleNormalReq();
}

void	Client::selectServer()
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
				this->server = new Server(*it);
				return ;
			}
		}
		this->server = new  Server(candidates[0]);
  }
}
