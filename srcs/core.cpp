#include "../inc/core.hpp"
#include <sys/socket.h>

Core::Core()
{
    this->parseMimeTypes();
}

Core::~Core()
{
    for (size_t i = 0; i < this->pollFds.size(); i++)
    {
        if (this->pollFds[i].fd != -1)
            close(this->pollFds[i].fd);
    }

    for (std::map<int, Client *>::iterator it = this->map_clients.begin(); it != this->map_clients.end(); it++)
        delete it->second;
    this->map_clients.clear();
    for (std::vector<SocketWrapper *>::iterator it = this->serverSockets.begin(); it != this->serverSockets.end(); it++)
        delete *it;
    this->serverSockets.clear();

    
    // for (size_t i = 0; i < this->serverSockets.size(); i++)
    // 	this->serverSockets[i].~SocketWrapper();
    // for (size_t i = 0; i < this->clients.size(); i++)
    // 	this->clients[i].~Client();
}

void Core::parseMimeTypes()
{
    std::string buffer;
    // std::ifstream file("/Users/yabtaour/Desktop/webserv-42/mime.types");
    std::ifstream file("mime.types");
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
        if (this->serverSockets[i]->get_sockfd() == fd)
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
        // std::cout << "ip:" << it->first << "  port:" << it->second << std::endl;
        // this->serverSockets.push_back(new SocketWrapper(AF_INET, SOCK_STREAM, 0));
        //  std::cout << "SocketWrapper done \n";
        // if (it->first == "NONE")
        //     this->serverSockets.back()->bind((*it).second);
        // else
        //     this->serverSockets.back()->bind((*it).first, (*it).second);
        // this->serverSockets.back()->listen(100);
        SocketWrapper sock(AF_INET, SOCK_STREAM, 0);
        if (it->first == "NONE")
            sock.bind(it->second);
        else
            sock.bind(it->first, it->second);
        sock.listen(100);
        this->serverSockets.push_back(new SocketWrapper(sock));
    }

    for (auto it : listens)
    {
        std::cout << GREEN << "ip : " << it.first << "  port : " << it.second << RESET << std::endl;
    }
}





bool Core::check_client_inactivity(Client &client , time_t timeout)
{
    time_t now = time(NULL);

    // if(client.request->connection.find("keep-alive") != std::string::npos)
        // return false;
    if (now - client.lastActivity > timeout )
    {
        std::cout << "Client with fd: " << client.fd  << " timed out\n";
        if ((client.request->connection.find("keep-alive") == std::string::npos))
            this->removeClient(client);
        // this->map_clients[client.fd]->session.SessionExpired = true;

        return true;
    }
    return false;
}



void Core::removeClient(Client &client)
{
    for (size_t i = 0; i < this->pollFds.size(); i++)
    {
        if (this->pollFds[i].fd == client.fd)
        {
            this->pollFds[i].fd = -1;
            close(client.fd);
            this->pollFds.erase(this->pollFds.begin() + i);
            // delete this->map_clients[client.fd];
            this->map_clients.erase(client.fd);
            i--;
            return;
        }
    }
}




void reset(Client *client)
{

    delete client->request;
    delete client->response;
    delete client->cgi;


    client->request = new Request();
    client->response = new Response();
    client->cgi = new Cgi();

    client->server = nullptr;
	client->location = nullptr;
    client->request->client_fd = client->fd;
    client->response->client_fd = client->fd;
    

}




void Core::handleConnections()
{   
    for (std::vector<SocketWrapper *>::iterator it = this->serverSockets.begin(); it != this->serverSockets.end(); it++)
    {
        // std::cout << "sockwrap address: " << *it << std::endl;
        pollfd fd;
        fd.fd = (*it)->get_sockfd();
        fd.events = POLLIN;
        fd.revents = 0;
        // std::cout << "fd.fd : " << fd.fd << std::endl;
        this->pollFds.push_back(fd);
    }
    
    while (true)
    {
        int ret = poll(this->pollFds.data(), this->pollFds.size(), 60);
        if (ret < 0)
            throw std::runtime_error("poll() failed");
 
        for (size_t i = 0; i < this->pollFds.size(); i++)
        {



            if (this->map_clients[this->pollFds[i].fd] &&  check_servers_socket(this->pollFds[i].fd) == -1)
            {
                if (this->map_clients[this->pollFds[i].fd]->request->state == Stat::DONE)
                {
                    reset(this->map_clients[this->pollFds[i].fd]);

                }
                if (this->map_clients[this->pollFds[i].fd]->response->GENERATE_RES )
                {

                    std::cout << "GENERATE_RES\n";
                    this->map_clients[this->pollFds[i].fd]->selectServer();


                    this->pollFds[i].events |= POLLOUT;
                    if (this->map_clients[this->pollFds[i].fd]->request->method.size() != 0)
                        this->map_clients[this->pollFds[i].fd]->generateResponse();
                    this->pollFds[i].events &= ~POLLOUT;
                }
                check_client_inactivity(*this->map_clients[this->pollFds[i].fd] , TIMEOUT);
            }

  
            
            if (this->pollFds[i].revents & POLLIN)
            {   


                if (check_servers_socket(this->pollFds[i].fd) != -1)
                {
                    int it = check_servers_socket(this->pollFds[i].fd);
                        // New connection
                    std::cout << "New connection\n";

                    this->clients.push_back(new Client(this->serverSockets[it]));
                    this->clients.back()->core = this;

                    this->map_clients.insert(std::make_pair(this->clients.back()->fd, this->clients.back()));

                    
                    this->pollFds.push_back(this->clients.back()->pollfd_);
                    // std::cout <<  "new fd : " << this->pollFds.back().fd << "size: " << this->pollFds.size() << std::endl;


                }
                else
                {	
                    try {
                            // std::cout << "handle request\n" << std::endl;
                            // std::cout <<  "new fd : " << this->pollFds.back().fd << "size: " << this->pollFds.size() << std::endl;
                            this->map_clients[this->pollFds[i].fd]->handleRequest();
                    }
                    catch(const std::exception& e)
                    {

                        Parser::lex()->set_input(e.what());
                        int code = atoi(Parser::lex()->next_token(false).c_str());

                        if (this->map_clients[this->pollFds[i].fd]->response->checkError(code))
                            this->map_clients[this->pollFds[i].fd]->response->responseStr = this->map_clients[this->pollFds[i].fd]->response->generateError(e.what(), DEFAULT);
                        else
                            this->map_clients[this->pollFds[i].fd]->response->responseStr = this->map_clients[this->pollFds[i].fd]->response->generateError(e.what(), MINE);

                        send(this->pollFds[i].fd, this->map_clients[this->pollFds[i].fd]->response->responseStr.c_str(), this->map_clients[this->pollFds[i].fd]->response->responseStr.size(), 0);
                        this->map_clients[this->pollFds[i].fd]->request->state = DONE;  
                    }
                }
            }
            
            else if (this->pollFds[i].revents & POLLHUP )
            {
                std::cout << "dissconected\n";
                this->pollFds[i].fd = -1;
                close(this->pollFds[i].fd);
                this->pollFds.erase(this->pollFds.begin() + i);
                // this->map_clients[this->pollFds[i].fd;
                delete this->map_clients[this->pollFds[i].fd];
                this->map_clients.erase(this->pollFds[i].fd);
                i--;

            }
            
            else if (this->pollFds[i].revents & POLLERR)
            {  
                std::cout << "error\n";
                this->pollFds[i].fd = -1;
                close(this->pollFds[i].fd);
                this->pollFds.erase(this->pollFds.begin() + i);
                delete this->map_clients[this->pollFds[i].fd];
                this->map_clients.erase(this->pollFds[i].fd);
                i--;
            }
        }
    }
}

