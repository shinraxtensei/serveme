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
    FIRSTLINE,
    HEADERS,
    BODY,
    DONE
};

class Request
{
    public:
        Request();
        Request(const Request &other);
        Request &operator=(const Request &other);
        ~Request();

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
			Client	*client; // this is a pointer to its parent client
			Http	*http;

			Response() {};
			~Response() {};

			void	checkAllowedMethods();
			void	matchLocation();
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