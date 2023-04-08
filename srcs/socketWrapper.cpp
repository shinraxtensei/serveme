#include "../inc/servme.hpp"
// #include "socketWrapper.hpp"

// default constructor
SocketWrapper::SocketWrapper()
{
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0)
    {
        throw std::runtime_error("Failed to create socket");
    }
    fcntl(sockfd_, F_SETFL, O_NONBLOCK);
}

SocketWrapper::SocketWrapper(int domain, int type, int protocol)
{
    sockfd_ = socket(domain, type, protocol);
    if (sockfd_ < 0)
    {
        throw std::runtime_error("Failed to create socket");
    }
    fcntl(sockfd_, F_SETFL, O_NONBLOCK);
    int reuse = 1;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

}

SocketWrapper::~SocketWrapper()
{
    // close(sockfd_);
}

void SocketWrapper::bind(int port)
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    int res = ::bind(sockfd_, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1)
    {
        std::cerr << "bind error: " << strerror(errno) << std::endl;
        throw std::runtime_error("Failed to bind socket");
    }
    this->listenPair.first = "NONE";
    this->listenPair.second = port;
}

void SocketWrapper::bind(std::string ip, int port)
{
 
    sockaddr_in addr;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0)
    {
        throw std::runtime_error("Invalid IP address");
    }
    // std::cout << "ip: " << ip << " port: " << port << std::endl;
    int res = ::bind(sockfd_, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1)
    {
        throw std::runtime_error(strerror(errno));
    }
    this->listenPair.first = ip;
    this->listenPair.second = port;
}

void SocketWrapper::listen(int backlog)
{
    int res = ::listen(sockfd_, backlog);
    if (res == -1)
    {
        throw std::runtime_error("Failed to listen on socket");
    }
}

int SocketWrapper::accept(sockaddr_in &client_addr)
{
    socklen_t addrlen = sizeof(client_addr);
    int clientfd = ::accept(sockfd_, (struct sockaddr *)&client_addr, &addrlen);
    if (clientfd == -1)
    {
        std::cerr << "accept error: " << strerror(errno) << std::endl;
        throw std::runtime_error("Failed to accept connection");
    }
    return clientfd;
}

// void SocketWrapper::connect(const char *ip_address, int port)
// {
//     sockaddr_in addr;
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(port);
//     if (inet_pton(AF_INET, ip_address, &addr.sin_addr) <= 0)
//     {
//         throw std::runtime_error("Invalid IP address");
//     }
//     int res = ::connect(sockfd_, (struct sockaddr *)&addr, sizeof(addr));
//     if (res == -1)
//     {
//         throw std::runtime_error("Failed to connect to server");
//     }
// }

int SocketWrapper::get_sockfd() const
{
    return sockfd_;
}

std::pair<std::string, int> SocketWrapper::get_listenPair() const
{
    return this->listenPair;
}

bool SocketWrapper::operator==(const SocketWrapper &other) const
{
    return sockfd_ == other.sockfd_;
}

bool SocketWrapper::operator!=(const SocketWrapper &other) const
{
    return sockfd_ != other.sockfd_;
}

bool SocketWrapper::operator<(const SocketWrapper &other) const
{
    return sockfd_ < other.sockfd_;
}

bool SocketWrapper::operator>(const SocketWrapper &other) const
{
    return sockfd_ > other.sockfd_;
}

bool SocketWrapper::operator<=(const SocketWrapper &other) const
{
    return sockfd_ <= other.sockfd_;
}

bool SocketWrapper::operator>=(const SocketWrapper &other) const
{
    return sockfd_ >= other.sockfd_;
}