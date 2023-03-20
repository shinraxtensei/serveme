#pragma once

#include "cgi.hpp"
#include "servme.hpp"





class Core;
class SocketWrapper;
class Cgi;

enum Stat
{
    FIRSTLINE,
    HEADERS,
    BODY,
    DONE
};

class Server;
class Client;
class Request
{
    public:
        Request();
        // copy constructor
        Request(const Request &other);
        // assignment operator
        Request &operator=(const Request &other);
        ~Request();

        // Http *http;
        Stat state;
        Core *core;
        Client *client; // this is a pointer to its parent client
        Server *server;
        std::string buffer; 

        std::stringstream ss;
        std::multimap<std::string, std::string > headers;
        std::ofstream body;
        std::string bodyString;



        std::string method;
        std::string url;
        std::string version;

        int contentLength;
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
        // void ParseBodyChunked();

        void selectServer();
        
    // int handle error();
};

class Response
{ };





class Client
{
    public:
        int fd;
        Core *core;
        pollfd pollfd_;
        sockaddr_in *addr; 

        Request *request;
        Cgi *cgi;
        // Response *response;
        SocketWrapper *socket;
    Client();
    ~Client();

    Client(SocketWrapper &socket);


        //**  methods
    void handleRequest();
    void cgi_handler();
    // void generateResponse();
    // void writeResponse();
    // void checkInactivity();


};