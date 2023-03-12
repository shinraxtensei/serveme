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

class Server;
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
        Server *server;
        std::string buffer; 
        std::stringstream ss; 
        std::map<std::string, std::string> headers;
        std::ofstream body;



        std::string method;
        std::string url;
        std::string version;

        std::string contentLength;
        std::string transferEncoding;
        std::string host;
        std::string connection;


        // std::map<std::string, std::string> mimeTypes;


        //** methods
        // void ParseRequest();
        std::string	checkType(std::string path);
        void ParseFirstLine(std::string &line);
        void ParseHeaders(std::string &line);
        void ParseBody();
        void ParseBodyChunked();

        void selectServer();
        
    // int handle error();
};

class Response
{};





class Client
{
    public:
        int fd;
        Core *core;
        pollfd pollfd_;
        sockaddr_in *addr; 
        Request *request;
        Response *response;
        SocketWrapper *socket;

    Client();
    ~Client();

    Client(SocketWrapper &socket);


        //**  methods
    
    void handleRequest();
    // void generateResponse();
    // void writeResponse();
    // void checkInactivity();


};