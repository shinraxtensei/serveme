#include "../inc/core.hpp"



Http *Core::get_http()
{
    return Parser::getHttp();
}



int  Core::check_servers_socket(int fd)
{
    // std::cout << "inside check_servers_socket - fd : " << fd << std::endl;
    // for (std::vector<SocketWrapper>::iterator it = this->serverSockets.begin(); it != Core::serverSockets.end(); it++)
    // {
    //     if (it->get_sockfd() == fd)
    //         return it;
    // }

    for (size_t i = 0; i < this->serverSockets.size(); i++)
    {   
        if (this->serverSockets[i].get_sockfd() == fd)
            return  i;
    }
    return -1;
}




void Core::startup()
{
    std::set< std::pair<std::string, int> > listens ;
    for(size_t i = 0; i < Parser::getHttp()->servers.size(); i++)
    {
        std::pair<std::string, int> listen;
        listen.first = Parser::getHttp()->servers[i].ipPort.first;
        listen.second = Parser::getHttp()->servers[i].ipPort.second;
        listens.insert(listen);
    }

    for (std::set< std::pair<std::string, int> >::iterator it = listens.begin(); it != listens.end(); it++)
    {
        SocketWrapper *sock = new SocketWrapper(AF_INET, SOCK_STREAM, 0);
        if (it->first == "NONE")
            sock->bind(it->second);
        else
            sock->bind(it->first, it->second);
        sock->listen(100);
        this->serverSockets.push_back(*sock);
    }

    for (auto it : listens)
    {
        std::cout << GREEN << "ip : " <<  it.first << "  port : " <<  it.second << RESET << std::endl;
    }
}



// TODO : make the funcion cleaner , add the handle response  (listne on pollout)
// TODO : make the clients able to disconnect
// TODO : create a client class that will store info about every client 
// TODO : now the servers dont have to have a socket wrapper , they only has to store the port and the socket fd
// TODO : the core should handle the creation and binding of sockets , and it should be able to give to the server to handle the appropriate request , depending on the servername/default server 

// void Core::handleConnections()
// {

//     std::vector<pollfd> pollFds;
//     for (size_t i = 0; i < Parser::getHttp()->servers.size(); i++)
//     {
//         Parser::getHttp()->servers[i].sock = new SocketWrapper(AF_INET, SOCK_STREAM, 0);
//         Parser::getHttp()->servers[i].connect();

//         pollfd fd;
//         fd.fd = Parser::getHttp()->servers[i].sock->get_sockfd();
//         fd.events = POLLIN | POLLHUP | POLLERR;
//         pollFds.push_back(fd);
//     }

//     while(true)
//     {
//         int ret = poll(pollFds.data(), pollFds.size(), 10);
//         if (ret == -1)
//         {
//             std::cerr << "Error: poll() failed" << std::endl;
//             exit(1);
//         }
//         for (size_t i = 0 ; i < pollFds.size() ; i++)
//         {   
//             if (pollFds[i].revents & POLLIN)
//             {
//                 if (int server_index = check_servers_socket(pollFds[i].fd); server_index != -1)
//                 {
//                     // then its server socket
//                     std::cout << "new connection\n" ;
//                     sockaddr_in client_addr;
//                     int client_fd = Parser::getHttp()->servers[server_index].sock->accept(client_addr);
//                     if (client_fd == -1)
//                     {
//                         std::cerr << "Error: accept() failed" << std::endl;
//                         continue;
//                     }
//                     fcntl(client_fd, F_SETFL, O_NONBLOCK);
//                     pollfd fd;
//                     fd.fd = client_fd;
//                     fd.events = POLLIN;
//                     pollFds.push_back(fd);
//                     Parser::getHttp()->servers[server_index].fds.push_back(client_fd);
//                 }
//                 else
//                 {
//                     for (size_t j = 0; j < Parser::getHttp()->servers.size(); j++)
//                     {
//                         if (Parser::getHttp()->servers[j].fds.end() != std::find(Parser::getHttp()->servers[j].fds.begin(), Parser::getHttp()->servers[j].fds.end(), pollFds[i].fd))
//                         {
//                             Parser::getHttp()->servers[j].HandleRequest(pollFds[i].fd);
//                             break;
//                         }
//                     }
//                 }
//                     // close(pollFds[i].fd);
//                     // pollFds.erase(pollFds.begin() + i);
//                     continue;
//             }
//             else if (pollFds[i].revents & POLLHUP)
//             {
//                 std::cout << "client disconnected \n";
//                 close(pollFds[i].fd);
//                 pollFds.erase(pollFds.begin() + i);
//                 // continue;
//             }
//         }
//     }
// }


void Core::HandleResquest(int fd)
{
    // TODO: we need to parse the request in order to check for the servername to see which server should handle the request


    std::cout << "handle request\n";
    std::string request;
    char buf[1024];
    int ret = 0;
    while ((ret = recv(fd, buf, 1024, 0)) > 0)
    {
        request.append(buf, ret);
        if (ret < 1024)
            break;
    }
    std::cout << request << std::endl;
    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World!";
    send(fd, response.c_str(), response.size(), 0);
}



void Core::handleConnections()
{
    std::vector<pollfd> pollFds;
    for (std::vector<SocketWrapper>::iterator it = this->serverSockets.begin(); it != this->serverSockets.end(); it++)
    {
        // std::cout << RED << "inside handle connections - fd :" << it->get_sockfd() << RESET << std::endl;
        pollfd fd;
        fd.fd = it->get_sockfd();
        fd.events = POLLIN | POLLHUP | POLLOUT;
        pollFds.push_back(fd);
    }

    while(true)
    {
        int ret = poll(pollFds.data(), pollFds.size(), 10);
        if (ret == -1)
        {
            std::cerr << "Error: poll() failed" << std::endl;
            exit(1);
        }

        for (size_t i = 0; i < pollFds.size() ; i++)
        {
            if (pollFds[i].revents & POLLIN)
            {
                if (int it = check_servers_socket(pollFds[i].fd) ; it != -1)
                {
                    // std::cout << "it : " << this->serverSockets[it].get_sockfd() << std::endl;
                    // then its server socket
                    std::cout << "new connection\n" ;

                    sockaddr_in client_addr;
                    int client_fd = this->serverSockets[it].accept(client_addr);
                    // std::cout << "client fd : " << client_fd << std::endl;
                    if (client_fd == -1)
                    {
                        std::cerr << "Error: accept() failed" << std::endl;
                        continue;
                    }
                    fcntl(client_fd, F_SETFL, O_NONBLOCK);
                    pollfd fd;
                    fd.fd = client_fd;
                    fd.events = POLLIN | POLLHUP | POLLOUT;
                    pollFds.push_back(fd);




                    // Client client(*it);
                    // pollFds.push_back(client.pollfd_);
                    // this clients vector is useless for now
                    // clients.push_back(Client(*it));
                }
                else
                {
                    Core::HandleResquest(pollFds[i].fd);
                    break;
                    // continue;
                }
            }
            else if (pollFds[i].revents & POLLHUP)
            {
                std::cout << "client disconnected \n";
                close(pollFds[i].fd);
                pollFds.erase(pollFds.begin() + i);
                // continue;
            }
        }
    }
}