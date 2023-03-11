#pragma once

#include "servme.hpp"



class Core;
class SocketWrapper;

enum Stat
{
    FIRSTLINE,
    HEADERS,
    BODY,
    DONE
};


class Request
{
    public:
        Request() {
            this->state = FIRSTLINE;
        };
        ~Request() {};

        // Http *http;
        Stat state;
        Core *core;
        std::string buffer; 
        std::stringstream ss; 
        std::map<std::string, std::string> headers;
        std::ofstream body;


        // std::map<std::string, std::string> mimeTypes;


        //** methods
        // void ParseRequest();
        std::string	checkType(std::string path);
        void ParseFirstLine();
        void ParseHeaders();
        void ParseBody();
        void ParseBodyChunked();
    // int handle error();
};

class Response
{};





class Client
{
    public:
        Core *core;
        int fd;
        pollfd pollfd_;
        sockaddr_in *addr; 
        Request *request;
        Response *response;

    Client();
    ~Client();

    Client(SocketWrapper &socket);


        //**  methods
    
    void handleRequest();
    // void generateResponse();
    // void writeResponse();
    // void checkInactivity();


};