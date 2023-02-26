#include "../inc/core.hpp"



Http *Core::get_http()
{
    return Parser::getHttp();
}




void Core::create_servers()
{
    // TODO : look for the listen directive convert it to int and bind it to the socket

    for (auto server : Parser::getHttp()->servers)
    {
        server.Socket();
        sockaddr_in client_addr;
        std::map<std::string , std::vector<std::string> >::iterator  it; 
        it = server.server_directives.find("listen");
        if (it != server.server_directives.end())
        {
            server.listen = std::stoi(it->second[0]);
        }
        else
        {
            std::cout << "Error: listen directive not found" << std::endl;
            exit(1);
        }
        server.sock->bind(server.listen);
        server.sock->listen(10);
        std::cout << "Server is listening on port " << server.listen << std::endl;
        std::cout << "Waiting for connections ..." << std::endl;
        int fd = server.sock->accept(client_addr);
        std::cout << "Connection accepted" << std::endl;
        std::cout << "Waiting for requests ..." << std::endl;
        char buffer[1024];
        read(fd, buffer, 1024);
        std::cout << "Request received" << std::endl;
        std::cout << "Request: " << buffer << std::endl;
        
    }
}


