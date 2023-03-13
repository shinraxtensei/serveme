#include "../inc/client.hpp"

Client::Client()
{
    this->addr = new sockaddr_in;
    // this->response = new Response();

    this->request.core = this->core;
    this->request.client = this;
    // this->response->core = this->core;
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



void Client::handleRequest()
{
    // std::cout << "handling request ..." << std::endl;
    if (this->request.state == FIRSTLINE || this->request.state == HEADERS)
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
        this->request.buffer += buffer[0];
        if (line.find("\n") != std::string::npos)
        {
            if (this->request.state == FIRSTLINE)
                this->request.ParseFirstLine(line);
            else if (this->request.state == HEADERS)
                this->request.ParseHeaders(line);
            line = "";
        }
    }
    else if (this->request.state == BODY)
    {
        char buffer[1024];
        if (recv(this->fd, buffer, 1024, 0) == -1)
        {
            // handle error
            std::cerr << "Error: recv() failed" << std::endl;
        }
        else if (recv(this->fd, buffer, 1024, 0) == 0)
        {
            // disconnection
            std::cout << "disconnection" << std::endl;
        }

        this->request.buffer += buffer;
        this->request.body << buffer;
        this->request.ParseBody();
        // generateResponse();
        // writeResponse();
    }
}