#include "../inc/core.hpp"



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







void Core::HandleResquest(pollfd FD)
{
    // TODO : we need to parse the request in order to check for the servername to see which server should handle the request
    // TODO : 

    std::cout << "handle request\n";
    std::string request;
    char buf[1024];
    int ret = 0;
    while ((ret = recv(FD.fd, buf, 1024, 0)) > 0)
    {
        if (ret == -1 || ret == 0)
        {
            if (ret == -1)
            {
                std::cerr << "Error: recv() failed" << std::endl;
                exit(1);
            }
            else
            {
                std::cerr << "Error: client disconnected" << std::endl;
                exit(1);
            }
        }
        request.append(buf, ret);
        if (ret < 1024)
            break;
    }

    std::string status = "HTTP/1.1 200 OK\r\n";
    std::string headers = "Content-Type: text/html\r\n";
    std::string body = "<html><body><h1>Hello, world!</h1></body></html>";
    std::string response = status + headers + "\r\n" + body;
    send(FD.fd, response.c_str(), response.size(), 0);

}




void Core::handleConnections()
{
    
//     // TODO : make the clients able to disconnect 
//     // TODO : the core should handle the creation and binding of sockets , and it should be able to give to the server to handle the appropriate request , depending on the servername/default server 


    std::vector<pollfd> pollFds;
    for (std::vector<SocketWrapper>::iterator it = this->serverSockets.begin(); it != this->serverSockets.end(); it++)
    {
        pollfd fd;
        fd.fd = it->get_sockfd();
        fd.events = POLLIN | POLLHUP | POLLOUT;
        pollFds.push_back(fd);
    }

//     while(true)
//     {
//         int ret = poll(pollFds.data(), pollFds.size(), 10);
//         if (ret == -1)
//         {
//             std::cerr << "Error: poll() failed" << std::endl;
//             exit(1);
//         }

//         for (size_t i = 0; i < pollFds.size() ; i++)
//         {
//             if (pollFds[i].revents & POLLIN)
//             {
//                 if (int it = check_servers_socket(pollFds[i].fd) ; it != -1)
//                 {
//                     std::cout << "new connection\n" ;



//                     this->clients.push_back(Client(this->serverSockets[it]));
//                     pollFds.push_back(this->clients.back().pollfd_);

//                 }
//                 else
//                 {   
//                     std::cout << "request\n";
//                     char buf[1024];
//                     recv(pollFds[i].fd, buf, 1024, 0);
//                     // read(pollFds[i].fd, buf, 1);

//                     break;
//                 }
//             }
//             // else if (pollFds[i].revents & POLLOUT)
//             // {
//             //     std::cout << "POLLOUT\n";
//             //     exit(0);
//             // }
//             // else if (pollFds[i].revents & POLLHUP)
//             // {

//             //     std::cout << "client disconnected \n";
//             //     pollFds[i].fd = -1;

//             // }


//         }
//     }


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
                pollFds.push_back(this->clients.back().pollfd_);
            } 
            else {
                // Client request
                std::cout << "Client request\n";
                char buf[1024];
                int bytes_received = recv(pollFds[i].fd, buf, 1024, 0);
                if (bytes_received == -1) {
                    // Error
                    std::cerr << "Error: recv() failed" << std::endl;
                    close(pollFds[i].fd);
                    pollFds.erase(pollFds.begin() + i);
                    i--;
                } 
                else if (bytes_received == 0) {
                    // Client disconnected
                    std::cout << "Client disconnected\n";
                    close(pollFds[i].fd);
                    pollFds.erase(pollFds.begin() + i);
                    i--;
                } 
                else {
                    std::cout << "Request: " <<  buf << std::endl;
                    send(pollFds[i].fd, "<html> <h1> hello </h1> </html>", 5, 0);
                    // Parse request and send response
                    // std::string request(buf, bytes_received);
                    // std::string response = handle_request(request);
                    // send(pollFds[i].fd, response.c_str(), response.length(), 0);
                    // std::cout << "Request: " << request << std::endl;
                    // request = "Request : " + request;
                    // send(pollFds[i].fd,request.c_str(), 5, 0);
                }
            }
        }
    }
}





}



