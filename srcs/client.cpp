#include "../inc/servme.hpp"

// #include "../inc/client.hpp"
// #include <algorithm>
// #include <cstddef>
// #include <cstdlib>
// #include <cstring>
// #include <fstream>
// #include <ios>
// #include <iostream>
// #include <ostream>
// #include <string>
// #include <sys/_types/_pid_t.h>
// #include <sys/fcntl.h>
// #include <sys/socket.h>


// #include <vector>

Client::Client()
{
    std::cout << " Client created without socket\n";
    this->lastActivity = time(NULL);

    this->addr = new sockaddr_in;

    this->request = new Request();
    this->request->core = this->core;
    this->request->client = this;

    this->location = nullptr;
    this->response = new Response();
    this->response->http = this->core->get_http();
    this->response->client = this;

    this->pollfd_.fd = -1;
}


// // deep copy constructor
// Client::Client(const Client &client)
// {
//     std::cout << "Client created with socket\n";
//     this->lastActivity = time(NULL);
//     this->fd = client.fd;
//     this->addr = new sockaddr_in;
//     this->addr->sin_addr.s_addr = client.addr->sin_addr.s_addr;
//     this->addr->sin_family = client.addr->sin_family;
//     this->addr->sin_port = client.addr->sin_port;
//     this->request = new Request(*client.request);
//     this->request->client = this;
//     this->location = client.location;
//     this->response = new Response(*client.response);
//     this->response->client = this;
//     this->pollfd_.fd = client.pollfd_.fd;
//     this->pollfd_.events = client.pollfd_.events;
//     this->pollfd_.revents = client.pollfd_.revents;
// }









Client::~Client()
{

    if (this->fd == -1)
    {
        std::cout << "Client destroyed without socket\n";
        // std::cout << "memory address" << this << std::endl;
    }
    else
    {
        std::cout << "Client destroyed with socket\n";
        // std::cout << "memory address" << this << std::endl;
    }
        

    if (this->pollfd_.fd != -1)
    {
        close(this->pollfd_.fd);
        this->pollfd_.fd = -1;
    }
    if (this->addr != nullptr)
    {
        delete this->addr;
        this->addr = nullptr;
    }
    if (this->request != nullptr)
    {
        delete this->request;
        this->request = nullptr;
    }
    if (this->response != nullptr)
    {
        delete this->response;
        this->response = nullptr;
    }


    if (this->cgi != nullptr)
    {
        delete this->cgi;
        this->cgi = nullptr;
    }
    if (this->server != nullptr)
    {
        delete this->server;
        this->server = nullptr;
    }
    if (this->location != nullptr)
    {
        delete this->location;
        this->location = nullptr;
    }
    // close(this->fd);
    // delete this->addr;
    // delete this->request;
    // delete this->response;
}
// std::string generateSessionId(size_t length) {
//   static const char charset[] =
//       "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
//   const size_t charset_size = sizeof(charset) - 1;
//   std::srand(std::time(0));
//   std::stringstream ss;
//   for (size_t i = 0; i < length; ++i) {
//     ss << charset[std::rand() % charset_size];
//   }
//   return ss.str();
// }
Client::Client(SocketWrapper *sock)
{
    
    std::cout << " Client created with socket\n";
    // std::cout << "memory address" << this << std::endl;
    this->lastActivity = time(NULL);


    this->session.user_id = std::to_string(this->fd);
    this->session.SessionExpired = false;
    this->session.session_id = generateSessionId(50);
    this->session.path = "/";
    this->session.Expires = time(NULL) + TIMEOUT;



    this->cgi = new Cgi();
    this->request = new Request();
    this->response = new Response();
    this->request->client = this;
    this->response->client = this;

    this->request->core = this->core;
    this->server = nullptr;
	this->location = nullptr;

    // this->response->http = this->core->get_http();

    this->socket = sock;
    addr = new sockaddr_in;
    fd = sock->accept(*addr);
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
    pollfd_.revents = 0;
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

    this->lastActivity = time(NULL);
    this->session.Expires = time(NULL) + TIMEOUT;




    if (this->request->state & (Stat::START | Stat::FIRSTLINE | Stat::HEADERS))
    {

        if (this->request->state & Stat::FIRSTLINE)
        {
    
        }
        static std::string line = "";
        char buffer[1];
        int ret;
        // ret = recv(this->fd, buffer, 1, 0);
        // std::cout << "this->fd" << this->fd << std::endl;
        ret = read(this->fd, buffer, 1);
        if (ret == -1)
        {
            std::cerr << "Error: recv() failed" << std::endl;
            return;
        }
        else if (ret == 0)
        {
            std::cout << "disconnection" << std::endl;
            this->core->removeClient(*this);
            // Servme::getCore()->map_clients[this->fd].pollfd_.fd = -1;
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
                // Client::handleCookies();
                this->response->GENERATE_RES = true;
                this->request->state = Stat::BODY;
                // if (this->request->method == "GET")
                    // this->request->state = Stat::END;
                // else
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
        std::cout << CYAN << "STATE: " << (this->request->state == BODY ? "BODY" : "weird") << RESET << std::endl;
        
        // if (this->request->state & Stat::END)
            // return;
        
        static int flag = 0;

        

        try {
            this->request->ParseBody();
        }
        catch (const std::exception& e)
        {

            if (std::string(e.what()) == "Disconnected")
            {
                std::cout << "disconnection" << std::endl;
                this->core->removeClient(*this);
            }
            // else
                std::cout << e.what() << std::endl;
        }

        if (this->request->bodyType == BodyType::CHUNKED)
        {
			// std::cout << "body type : chunked" << std::endl;
			
            if (flag == 0)
            {
                flag = 1;
                std::cout << BLUE <<"this is the start of chunked body" << RESET << std::endl;
                this->request->state = Stat::CHUNKED_START;
            }
            this->request->ParseChunkedBody(); 
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
            this->request->ParseMultiPartBody(); 
        }
 
    }


}



void Client::generateResponse()
{
	std::cout << "in generateResponse" << std::endl;
	// this->response->client = Servme::getCore()->map_clients[this->response->client_fd];
    std::cout << "state: " << this->request->state << std::endl;
    std::cout << "this->request->uri : " << this->request->url << std::endl;
    std::cout << "this->request->method : " << this->request->method << std::endl;


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


    for (it = Servme::getCore()->get_http()->servers.begin();  it != Servme::getCore()->get_http()->servers.end(); it++)
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
