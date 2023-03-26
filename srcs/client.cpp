#include "../inc/client.hpp"

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
                    this->request->state = BODY; // wtf is this
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
            std::cerr << "Error: recv() failed" << std::endl;
            return;
        }
        else if (ret == 0)
        {
            std::cout << "disconnection" << std::endl;
            return;
        }
        this->request->bodyString += std::string(buffer, ret);
        this->request->buffer += std::string(buffer, ret);
        // this->request->ParseBody();
        this->generateResponse();
        // writeResponse();
    }
}

void	Client::generateResponse()
{
	this->response->client = &Servme::getCore()->map_clients[this->response->client_fd];
	this->response->checkAllowedMethods();
	this->response->checkCgi();
	if (this->cgiFlag == 1)
	{
		// cgi matching
	}
	else
		this->response->matchLocation(this->server->locations);
	this->path = this->location->root + this->request->url;
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
				this->server = new  Server(*it);
				return ;
			}
		}
		this->server = new  Server(candidates[0]);
	}
}