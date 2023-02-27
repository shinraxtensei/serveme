#include "../inc/core.hpp"



Http *Core::get_http()
{
    return Parser::getHttp();
}




// void Core::create_servers()
// {
//     // TODO : look for the listen directive convert it to int and bind it to the socket

//     for (auto server : Parser::getHttp()->servers)
//     {
//         server.Socket();
//         sockaddr_in client_addr;
//         std::map<std::string , std::vector<std::string> >::iterator  it; 
//         it = server.server_directives.find("listen");
//         if (it != server.server_directives.end())
//         {
//             server.listen = std::stoi(it->second[0]);
//         }
//         else
//         {
//             std::cout << "Error: listen directive not found" << std::endl;
//             exit(1);
//         }
//         server.sock->bind(server.listen);
//         server.sock->listen(10);
//         std::cout << "Server is listening on port " << server.listen << std::endl;
//         std::cout << "Waiting for connections ..." << std::endl;
//         int fd = server.sock->accept(client_addr);
//         std::cout << "Connection accepted" << std::endl;
//         std::cout << "Waiting for requests ..." << std::endl;
//         char buffer[1024];
//         read(fd, buffer, 1024);
//         std::cout << "Request received" << std::endl;
//         std::cout << "Request: " << buffer << std::endl;
        
//     }
// }


int check_servers_socket(int fd)
{
    for (size_t i = 0 ; i < Parser::getHttp()->servers.size(); i++)
    {
        if (Parser::getHttp()->servers[i].sock->get_sockfd() == fd)
            return 0;
    }
    return -1;
}


void Core::handleConnections()
{
    // std::multimap<int, pollfd > fds;

    std::vector<pollfd> pollFds;
    for(size_t i = 0 ; i < Parser::getHttp()->servers.size() ; i++)
    {
        Parser::getHttp()->servers[i].connect();
        
        pollfd fd;
        fd.fd = Parser::getHttp()->servers[i].sock->get_sockfd();
        fd.events = POLLIN;
        pollFds.push_back(fd);
    }
                    

        
    while(1)
    {
        
        int ret = poll((pollFds.data()), pollFds.size(), -1);
        if (ret == -1)
        {
            std::cout << "Error: poll() failed" << std::endl;
            exit(1);
        }
        
        for (size_t i = 0; i < pollFds.size(); i++)
        {
            if (pollFds[i].revents & POLLIN)
            {
                if( -1 == check_servers_socket(pollFds[i].fd))
                {
                    
                    // this means that its a client socket
                    for (size_t i = 0 ; i < Parser::getHttp()->servers.size(); i++)
                    {
                        if (Parser::getHttp()->servers[i].fds.end() != std::find(Parser::getHttp()->servers[i].fds.begin(), Parser::getHttp()->servers[i].fds.end(), pollFds[i].fd))
                        {
                            Parser::getHttp()->servers[i].HandleRequest();
                            // Parser::getHttp()->servers[i].HandleResponse();
                        }
                    }
                }
                else
                {
                    // this means that its a server socket
                                                                                // ? im not sure if i should store this sockadd_in
                    std::cout << "new connection" << std::endl;
                    sockaddr_in client_addr;
                    int server_index = check_servers_socket(pollFds[i].fd);
                    int client_fd = Parser::getHttp()->servers[server_index].sock->accept(client_addr);
                    pollfd fd;
                    fd.fd = client_fd;
                    fd.events = POLLIN;
                    pollFds.push_back(fd);
                    Parser::getHttp()->servers[server_index].fds.push_back(client_fd);
                }
            }
        }
    }
}