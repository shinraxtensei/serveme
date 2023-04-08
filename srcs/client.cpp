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
        // ret = recv(this->fd, buffer, 1, 0);
        ret = read(this->fd, buffer, 1);
        if (ret == -1)
        {
            std::cerr << "Error: recv() failed" << std::endl;
            return;
        }
        else if (ret == 0)
        {
            this->pollfd_.fd = -1;
            std::cout << "disconnection" << std::endl;
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
                // if (this->request->method == "GET")
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
			// std::cout << "body type : chunked" << std::endl;
			
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
			std::cout << "state : " << this->request->state << std::endl;
			std::cout << "body type : multipart" << std::endl;
            if (flag == 0)
            {
                flag = 1;
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
		// Servme::getCore()->pollFds.
		
		// this->pollfd_.events &= ~POLLIN;
		// this->pollfd_.events &= ~POLLOUT;
	}

    // if (this->response->GENERATE_RES || this->request->state == BODY)
	// {
	// 	std::cout << "here " << std::endl;
    //     this->generateResponse();
	// }
	// std::cout << "rj3na lhna" << std::endl;
}



void Client::generateResponse()
{
	std::cout << "in generateResponse" << std::endl;
	std::cout << "request state : " << this->request->state << std::endl;
	this->response->client = &Servme::getCore()->map_clients[this->response->client_fd];
	// this->response->checkAllowedMethods(); // error here aborted
	this->response->checkCgi();
	if (this->cgiFlag == 1)
        cgi_handler();
	else
		// this->response->handleNormalReq();
	std::cout << "done with generate response" << std::endl;
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
