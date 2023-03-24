#pragma once

#include "cgi.hpp"
#include "servme.hpp"

class Core;
class SocketWrapper;
class Cgi;
class Http;
class Server;
class Client;
class Location;


enum BodyType
{
    NONE,
    CHUNKED,
    MULTIPART
};




enum Stat
{
    // these are the state of the request
    START = 1 << 0,
    FIRSTLINE = 1 << 1,
    HEADERS = 1 << 2,
    // BODY = 1 << 3, this move to be a combined state
    DONE = 1 << 4,
    // this is for chunked encoding
    
    CHUNKED_SIZE = 1 << 5,
    CHUNKED_DATA = 1 << 6, // for both chunked and multipart

    // this is for multipart
    BOUNDARY = 1 << 7,
    MULTI_PART_HEADERS = 1 << 8,
    MULTI_PART_DATA = 1 << 9,
    END = 1 << 10 ,// for both chunked and multipart

    // combined states
    BODY =  CHUNKED_SIZE | CHUNKED_DATA | BOUNDARY | MULTI_PART_HEADERS | END
};







class Request
{
    public:
        Request();
        Request(const Request &other);
        Request &operator=(const Request &other);
        ~Request();

        int client_fd;
        Stat state;
        BodyType bodyType;
        Core *core;
        Client *client; // this is a pointer to its parent client
        std::string buffer; 

        std::stringstream ss;
        std::multimap<std::string, std::string > headers;
        std::ofstream body;
        std::string bodyString;

        std::string method;
        std::string url;
        std::string version;

        int 		contentLength;
        std::string transferEncoding;
        std::string host;
        std::string connection;

        //** methods
        std::string	checkType(std::string path);
        void ParseFirstLine(std::string &line);
        void ParseHeaders(std::string &line);
        void ParseBody();
        void ParseChunkedBody();
        void ParseMultiPartBody();
        // void ParseBodyChunked();
		// int handle error();
};

class Response
{
		public:
			int client_fd;
			Client	*client; // this is a pointer to its parent client
			Http	*http;

			Response() {};
			~Response() {};

			void	checkAllowedMethods();
			void	matchLocation();
			void	checkCgi();
};

class Client
{
    public:
        int				fd;
        Core			*core;
        pollfd			pollfd_;
        sockaddr_in		*addr; 

        Request			*request;
        Response		*response;
        SocketWrapper	*socket;
		Server			*server;
		Location		*location; // need to get the location path in config parsing

		int				cgiFlag;

        Cgi *cgi;
        // Response *response;
        // SocketWrapper *socket;
        Client();
        ~Client();
		std::string		path;
		Client(SocketWrapper &socket);


        //**  methods
    void handleRequest();
    void cgi_handler();
    // void generateResponse();
    // void writeResponse();
    // void checkInactivity();
		void	selectServer();
    	void	generateResponse();
        
    	// void writeResponse();
    	// void checkInactivity();


};