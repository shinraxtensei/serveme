#include "../inc/core.hpp"
#include <sys/socket.h>

Core::Core()
{
    this->parseMimeTypes();
}

Core::~Core()
{
    // for (size_t i = 0; i < this->serverSockets.size(); i++)
    // 	this->serverSockets[i].~SocketWrapper();
    // for (size_t i = 0; i < this->clients.size(); i++)
    // 	this->clients[i].~Client();
}

void Core::parseMimeTypes()
{
    std::string buffer;
    // std::ifstream file("/Users/yabtaour/Desktop/webserv-42/mime.types");
    std::ifstream file("/Users/ahouari/triz_work/serveme/mime.types");
    // if (!file.is_open())
    // {
    //     std::cout << RED << "Error opening mime.types file" << RESET << std::endl;
    //     exit(1);
    // }
    while (std::getline(file, buffer))
    {
        if (buffer.size() > 0 && buffer[0] != '#')
        {
            std::pair<std::string, std::string> pair;
            std::istringstream iss(buffer);
            iss >> pair.first;
            iss >> pair.second;
            this->mimeTypes.insert(pair);
        }
    }
}

Http *Core::get_http()
{
    return Parser::getHttp();
}

int Core::check_servers_socket(int fd)
{

    for (size_t i = 0; i < this->serverSockets.size(); i++)
    {
        if (this->serverSockets[i].get_sockfd() == fd)
            return i;
    }
    return -1;
}

void Core::startup()
{

    std::set<std::pair<std::string, int>> listens;
    for (size_t i = 0; i < Parser::getHttp()->servers.size(); i++)
    {
        std::pair<std::string, int> listen;
        listen.first = Parser::getHttp()->servers[i].ipPort.first;
        listen.second = Parser::getHttp()->servers[i].ipPort.second;
        listens.insert(listen);
    }

    for (std::set<std::pair<std::string, int> >::iterator it = listens.begin(); it != listens.end(); it++)
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
        std::cout << GREEN << "ip : " << it.first << "  port : " << it.second << RESET << std::endl;
    }
}





// bool Core::check_client_inactivity(Client &client , time_t timeout)
// {
//     time_t now = time(NULL);

//     if (now - client.lastActivity > timeout)
//     {
//         std::cout << "Client with fd: timed out\n";
//         return true;
//         // TODO: handle client timeout
//         // this->pollFds[getFromPollFds(this->pollFds, client.fd)].fd = -1;
//         // return;
//         // this->pollFds[getFromPollFds(this->pollFds, client->fd)].fd = -1;
//         // this->clients.erase(client);
//     }
//     return false;
// }











void Core::handleConnections()
{
    for (std::vector<SocketWrapper>::iterator it = this->serverSockets.begin(); it != this->serverSockets.end(); it++)
    {
        pollfd fd;

        fd.fd = it->get_sockfd();
        fd.events = POLLIN;
        this->pollFds.push_back(fd);
    }
    while (true)
    {
        int ret = poll(this->pollFds.data(), this->pollFds.size(), 10);
        if (ret == -1)
        {
            std::cerr << "Error: poll() failed" << std::endl;
            exit(1);
        }
        for (size_t i = 0; i < this->pollFds.size(); i++)
        {

            // if (check_client_inactivity(this->map_clients[this->pollFds[i].fd], 10))
            // {
            //     this->pollFds[i].fd = -1;
            // }



            if (this->map_clients[this->pollFds[i].fd].response->GENERATE_RES)
                this->map_clients[this->pollFds[i].fd].generateResponse();
                
            if (this->pollFds[i].revents & POLLIN)
            {
                if (int it = check_servers_socket(this->pollFds[i].fd); it != -1)
                {
                    // New connection
                    std::cout << "New connection\n";

                    this->clients.push_back(Client(this->serverSockets[it]));
                    this->clients.back().core = this; //****** gettin the core
                    
                    this->map_clients.insert(std::make_pair(this->clients.back().fd, this->clients.back()));

                    this->pollFds.push_back(this->clients.back().pollfd_);
                }
                else
                {	
                    try {
                        this->map_clients[this->pollFds[i].fd].handleRequest();
                    }
                    catch(const std::exception& e)
                    {

                        Parser::lex()->set_input(e.what());
                        int code = atoi(Parser::lex()->next_token(false).c_str());

                        if (this->map_clients[this->pollFds[i].fd].response->checkError(code))
                            this->map_clients[this->pollFds[i].fd].response->responseStr = this->map_clients[this->pollFds[i].fd].response->generateError(e.what(), DEFAULT);
                        else
                            this->map_clients[this->pollFds[i].fd].response->responseStr = this->map_clients[this->pollFds[i].fd].response->generateError(e.what(), MINE);

                        send(this->pollFds[i].fd, this->map_clients[this->pollFds[i].fd].response->responseStr.c_str(), this->map_clients[this->pollFds[i].fd].response->responseStr.size(), 0);
                        this->map_clients[this->pollFds[i].fd].request->state = DONE;
                        
                    }
                }
            }

            else if (this->pollFds[i].revents & POLLHUP)
            {
                std::cout << "Client disconnected\n";
                this->pollFds[i].fd = -1;
                // this->pollFds.erase(this->pollFds.begin() + i);
                // this->clients.erase(this->clients.begin() + i);
            }
            else if (this->pollFds[i].revents & POLLERR)
            {
                std::cout << "Error\n";
                this->pollFds[i].fd = -1;
                // this->pollFds.erase(this->pollFds.begin() + i);
                // this->clients.erase(this->clients.begin() + i);
            }
        }
    }
}
