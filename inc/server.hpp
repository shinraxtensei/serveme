#pragma once
#include "parser.hpp"
#include "socketWrapper.hpp"
#include "core.hpp"




class Client
{
    public:
        int fd;
        pollfd pollfd_;
        sockaddr_in *addr;
    
    Client(SocketWrapper socket){
        addr = new sockaddr_in;

        fd = socket.accept(*addr);
        if (fd == -1) {
            throw std::runtime_error("Failed to accept connection");
        }
        fcntl(fd, F_SETFL, O_NONBLOCK);
        pollfd_.fd = fd;
        pollfd_.events = POLLIN;
        
    }
};