#include "core.hpp"

class SocketWrapper {
public:
    SocketWrapper(int domain, int type, int protocol) {
        sockfd_ = socket(domain, type, protocol);
        if (sockfd_ < 0) {
            throw std::runtime_error("Failed to create socket");
        }
        fcntl(sockfd_, F_SETFL, O_NONBLOCK);
    }

    ~SocketWrapper() {
        close(sockfd_);
    }

    void bind(int port) {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        int res = ::bind(sockfd_, (struct sockaddr*)&addr, sizeof(addr));
        if (res == -1) 
        {
            std::cerr << "bind error: " << strerror(errno) << std::endl;
            throw std::runtime_error("Failed to bind socket");
        }
        // if (res == -1) {
        //     throw std::runtime_error("Failed to bind socket \n socket already in use");
        // }
    }

    void listen(int backlog) {
        int res = ::listen(sockfd_, backlog);
        if (res == -1) {
            throw std::runtime_error("Failed to listen on socket");
        }
    }

    int accept(sockaddr_in& client_addr) {
        socklen_t addrlen = sizeof(client_addr);
        int clientfd = ::accept(sockfd_, (struct sockaddr*)&client_addr, &addrlen);
        if (clientfd == -1) {
            throw std::runtime_error("Failed to accept connection");
        }
        return clientfd;
    }

    void connect(const char* ip_address, int port) {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        if (inet_pton(AF_INET, ip_address, &addr.sin_addr) <= 0) {
            throw std::runtime_error("Invalid IP address");
        }
        int res = ::connect(sockfd_, (struct sockaddr*)&addr, sizeof(addr));
        if (res == -1) {
            throw std::runtime_error("Failed to connect to server");
        }
    }

    int get_sockfd() const {
        return sockfd_;
    }

private:
    int sockfd_;
};
