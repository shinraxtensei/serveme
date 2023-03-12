#include "../inc/core.hpp"


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



int  Core::check_servers_socket(int fd)
{

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



void Core::handleConnections()
{


    // std::map<int , Client> map_clients;
    std::vector<pollfd> pollFds;
    std::unordered_map<int, std::string> buffers; // TODO : these will get replaced with ones for each request 
    std::unordered_map<int, std::chrono::steady_clock::time_point> lastInteraction; // TODO : these will get replaced with ones for each client


    // TODO : we need to map each fd with its client 

    for (std::vector<SocketWrapper>::iterator it = this->serverSockets.begin(); it != this->serverSockets.end(); it++)
    {
        pollfd fd;
        fd.fd = it->get_sockfd();
        fd.events = POLLIN;
        setsockopt(fd.fd, SOL_SOCKET, SO_REUSEADDR, NULL, 0);
        pollFds.push_back(fd);
    }

    while (true) {
        int ret = poll(pollFds.data(), pollFds.size(), 10);
        if (ret == -1) {
            std::cerr << "Error: poll() failed" << std::endl;
            exit(1);
        }

        for (size_t i = 0; i < pollFds.size(); i++) {
            if (pollFds[i].revents & POLLIN) {
                if (int it = check_servers_socket(pollFds[i].fd); it != -1) 
                {
                    // New connection
                    std::cout << "New connection\n";

                    this->clients.push_back(Client(this->serverSockets[it]));
                    this->clients.back().core = this; //****** gettin the core

                    this->map_clients.insert(std::pair<int , Client> (this->clients.back().fd , this->clients.back()));


                    pollFds.push_back(this->clients.back().pollfd_);
                    // buffers[this->clients.back().get_sockfd()] = "";
                    buffers[this->clients.back().fd] = "";
                    lastInteraction[this->clients.back().fd] = std::chrono::steady_clock::now();
                } 
                else {

                    this->map_clients[pollFds[i].fd].handleRequest();


                    // Client request
                    // std::cout << "Client request\n";
                    // char buf[1024];
                    // int bytes_received = recv(pollFds[i].fd, buf, 1024, 0); //!
                    // if (bytes_received == -1) {
                    //     // Error
                    //     std::cerr << "Error: recv() failed" << std::endl;
                    //     close(pollFds[i].fd);
                    //     pollFds.erase(pollFds.begin() + i);
                    //     i--;
                    // } 
                    // else if (bytes_received == 0) {
                    //     // Client disconnected
                    //     std::cout << "Client disconnected\n";
                    //     close(pollFds[i].fd);
                    //     pollFds.erase(pollFds.begin() + i);
                    //     buffers.erase(pollFds[i].fd);
                    //     lastInteraction.erase(pollFds[i].fd);
                    //     i--;
                    // } 
                    // else {
                    //     // std::cout << "Received " <<  bytes_received << " bytes from socket " << pollFds[i].fd << std::endl;
                    //     buffers[pollFds[i].fd] += std::string(buf, bytes_received);
                    //     lastInteraction[pollFds[i].fd] = std::chrono::steady_clock::now();

                    //     if (buffers[pollFds[i].fd].find("\r\n\r\n0\r\n\r\n") != std::string::npos ||
                    //         buffers[pollFds[i].fd].find("\r\n\r\n") != std::string::npos) 
                    //     {
                    //         // We have a complete request
                    //         std::cout << "Complete request received from socket " << pollFds[i].fd << ": " << std::endl;

                    //         std::cout<< GREEN << buffers[pollFds[i].fd] << RESET << std::endl;
                    //         // std::string response = handle_request(buffers[pollFds[i].fd]);
                    //         // send(pollFds[i].fd, response.c_str(), response.length(), 0);
                    //         buffers[pollFds[i].fd] = "";
                    //     }
                    // }
                }
            }
 

            // for (size_t i = 0 ; i < this->clients.size() ; i++)
            // {
            //     auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - lastInteraction[this->clients[i].fd]);
            //     if (elapsed_time.count() >= 100) 
            //     {
            //         std::cout << "closing connection with client, because of inactivity :\n";
            //         close(this->clients[i].fd);
            //         pollFds.erase(std::remove_if(pollFds.begin(), pollFds.end(), [this, i](const pollfd& fd) { return fd.fd == this->clients[i].fd; }), pollFds.end());
            //         this->clients.erase(this->clients.begin() + i);
            //         i--;
            //     }
            // }

        }

    }
}

