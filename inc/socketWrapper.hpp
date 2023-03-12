#pragma once

#include "servme.hpp"

// #include "core.hpp"

class SocketWrapper
{
    public:
        SocketWrapper();
        SocketWrapper(int domain, int type, int protocol);

    ~SocketWrapper() ;


    void bind(int port) ;
    void bind(std::string ip , int port);
    void listen(int backlog);
    int accept(sockaddr_in& client_addr) ;
    void connect(const char* ip_address, int port) ;
    int get_sockfd() const ;
    std::pair<std::string , int> get_listenPair() const;

    bool operator==(const SocketWrapper& other) const ;
    bool operator!=(const SocketWrapper& other) const ;
    bool operator<(const SocketWrapper& other) const ;
    bool operator>(const SocketWrapper& other) const;
    bool operator<=(const SocketWrapper& other) const ;
    bool operator>=(const SocketWrapper& other) const ;
  



private:
    int sockfd_;
    std::pair<std::string, int> listenPair;
    // int listenDirective;
};
