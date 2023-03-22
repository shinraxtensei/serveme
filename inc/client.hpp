#pragma once

#include "servme.hpp"

class Core;
class SocketWrapper;
class Http;
class Server;
class Client;
class Location;


enum Stat
{
    // these are the state of the request
    START,
    FIRSTLINE,
    HEADERS,
    BODY,
    DONE,
    // this is for chunked encoding
    
    SIZE,
    DATA, // for both chunked and multipart

    // this is for multipart
    BOUNDARY,
    MULTI_PART_HEADERS,
    
    END // for both chunked and multipart

    
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

		std::string		path;

    	Client();
		Client(SocketWrapper &socket);
    	~Client();


        //**  methods
		void	selectServer();
    	void	handleRequest();
    	void	generateResponse();
    	// void writeResponse();
    	// void checkInactivity();


};