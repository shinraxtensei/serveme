#include "../inc/core.hpp"


SocketWrapper *Server::Socket()
{
    if (this->sock == nullptr)
        this->sock = new SocketWrapper(AF_INET , SOCK_STREAM , 0);
    return this->sock;
}


Server::Server()
{
    // Socket();
}

Server::~Server()
{
    // delete this->sock;
}

void Server::connect()
{
    // TODO : handle the request
    
    try
    {    
        Socket();
        if (this->server_directives.find("listen") != this->server_directives.end())
        {
            std::cout << "listen : " << this->server_directives["listen"][0]  << std::endl;
            this->listen = std::stoi(this->server_directives["listen"][0]);
            this->sock->bind(this->listen);
        }
        else
        {
            std::cout << "Error: listen directive not found" << std::endl;
            exit(1);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void Server::HandleRequest()
{
std::cout << "HandleRequest" << std::endl;
}

