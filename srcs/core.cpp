#include "../inc/core.hpp"



Http *Core::get_http()
{
    return Parser::getHttp();
}



int check_servers_socket(int fd)
{
    for (size_t i = 0 ; i < Parser::getHttp()->servers.size(); i++)
    {
        if (Parser::getHttp()->servers[i].sock->get_sockfd() == fd)
            return i;
    }
    return -1;
}


void Core::handleConnections()
{

    std::vector<pollfd> pollFds;
    for (size_t i = 0; i < Parser::getHttp()->servers.size(); i++)
    {
        Parser::getHttp()->servers[i].sock = new SocketWrapper(AF_INET, SOCK_STREAM, 0);
        Parser::getHttp()->servers[i].connect();

        pollfd fd;
        fd.fd = Parser::getHttp()->servers[i].sock->get_sockfd();
        fd.events = POLLIN;
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
        for (size_t i = 0 ; i < pollFds.size() ; i++)
        {
            if (pollFds[i].revents & POLLIN)
            {
                
                if (int server_index = check_servers_socket(pollFds[i].fd); server_index != -1)
                {
                    // then its server socket
                    std::cout << "new connection\n" ;
                    sockaddr_in client_addr;
                    int client_fd = Parser::getHttp()->servers[server_index].sock->accept(client_addr);
                    if (client_fd == -1)
                    {
                        std::cerr << "Error: accept() failed" << std::endl;
                        continue;
                    }
                    fcntl(client_fd, F_SETFL, O_NONBLOCK);
                    pollfd fd;
                    fd.fd = client_fd;
                    fd.events = POLLIN;
                    pollFds.push_back(fd);
                    Parser::getHttp()->servers[server_index].fds.push_back(client_fd);
                }
                else
                {
                    for (size_t j = 0; j < Parser::getHttp()->servers.size(); j++)
                    {
                        if (Parser::getHttp()->servers[j].fds.end() != std::find(Parser::getHttp()->servers[j].fds.begin(), Parser::getHttp()->servers[j].fds.end(), pollFds[i].fd))
                        {
                            Parser::getHttp()->servers[j].HandleRequest(pollFds[i].fd);
                            break;
                        }

                    }

                    // close(pollFds[i].fd);
                    // pollFds.erase(pollFds.begin() + i);
                    continue;
                }
            }
        }
    }
}