#include "../inc/core.hpp"


// SocketWrapper *Server::Socket()
// {
//     if (this->sock == nullptr)
//         this->sock = new SocketWrapper(AF_INET , SOCK_STREAM , 0);
//     std::cout << this->sock->get_sockfd() << std::endl;
//     return this->sock;
// }


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
    try
    {    
        
        if (this->server_directives.find("listen") != this->server_directives.end())
        {
            this->listen = std::stoi(this->server_directives["listen"][0]);
            std::cout << "listen: " << this->listen << std::endl;
            this->sock->bind(this->listen);
        }
        else
        {
            std::cout << "Error: listen directive not found" << std::endl;
            exit(1);
        }
        this->sock->listen(10);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void Server::HandleRequest( int fd)
{

    char buffer[1024];
    read(fd, buffer, 1024);
    std::cout << "Request received" << std::endl;
    std::cout << "Request: " << buffer << std::endl;

	std::cout << "test" << std::endl;

}


void Server::HandleResponse() {}
