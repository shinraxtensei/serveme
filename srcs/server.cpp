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
    delete this->sock;
}

void Server::HandleConnection()
{
    Socket();
    std::map<std::string , std::vector<std::string> >::iterator  it; 
    it = server_directives.find("listen");
    if (it != server_directives.end())
        this->listen = std::stoi(it->second[0]);
    else
    {
        std::cout << "Error: listen directive not found" << std::endl;
        exit(1);
    }
    this->sock->bind(this->listen);


}
