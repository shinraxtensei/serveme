#pragma once

#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <signal.h>
#include <string.h>
#include "parser.hpp"
#include "socketWrapper.hpp"
#include "server.hpp"

class Http;




enum Stat
{
    FIRSTLINE,
    HEADERS,
    BODY,
    DONE
};





class Request
{

    Request(Client &client)
    {
        this->client = client;
        this->status = FIRSTLINE;
    };

    // Http *http;
    Client client;
    Stat status;
    std::string buffer; 
    std::stringstream ss; 
    std::map<std::string, std::string> headers;
    std::ofstream body;


    void ParseRequest();
    void ParseFirstLine();
    void ParseHeaders();
    void ParseBody();
    void ParseBodyChunked();
    // int handle error();
};




class Response
{

};



class Core
{

    public:

    Core(){};
    
    ~Core()
    {
        for (size_t i = 0; i < this->serverSockets.size(); i++)
            this->serverSockets[i].~SocketWrapper();
        for (size_t i = 0; i < this->clients.size(); i++)
            this->clients[i].~Client();
    }


        // std::multimap<Request> requests;
        // std::vector<Request> requests;
        std::vector<SocketWrapper> serverSockets;
        std::vector<Client> clients;
        int check_servers_socket(int fd);
        void handleConnections();
        void HandleResquest(pollfd FD);
        Http *get_http();
        void startup();
        void checkInactivity();
};