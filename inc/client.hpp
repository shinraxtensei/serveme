#pragma once

#include "cgi.hpp"
#include "servme.hpp"
#include <fcntl.h>
#include <unistd.h>

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

#define FILE 1
#define DIRE 2

#define DEFAULT 1
#define MINE 2

enum Stat
{
    // these are the state of the request
    START = 1 << 0,
    FIRSTLINE = 1 << 1,
    HEADERS = 1 << 2,
    // BODY = 1 << 3, this moved to be a combined state
    DONE = 1 << 4,
    // this is for chunked encoding
    
    CHUNKED_START = 1 << 5,
    CHUNKED_SIZE = 1 << 6,
    CHUNKED_DATA = 1 << 7, // for both chunked and multipart

    // this is for multipart
    MULTI_PART_START = 1 << 8,
    MULTI_PART_BOUNDARY = 1 << 9,
    MULTI_PART_HEADERS = 1 << 10,
    MULTI_PART_DATA = 1 << 11,

    END = 1 << 12 ,// for both chunked and multipart

    // combined states
    BODY = (CHUNKED_START | CHUNKED_SIZE | CHUNKED_DATA  | MULTI_PART_START | MULTI_PART_BOUNDARY | MULTI_PART_HEADERS | MULTI_PART_DATA | END)
};



struct Session
{
    std::string session_id;
    std::string user_id;
    bool SessionExpired;

    std::string path;
    std::string Expires;
    std::string MaxAge;

};

struct Multipart_ENV
{
    
    Multipart_ENV();
    Multipart_ENV(std::string fileName , std::string contentType, std::string data);
    Multipart_ENV(std::string fileName , std::string contentType);
    ~Multipart_ENV();
    std::string field_name;
    std::string file_name;
    std::string content_type;
    std::string data;
    unsigned long pos;
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
        std::map<std::string, Multipart_ENV> multipart_env;
        BodyType bodyType;
        Core *core;
        Client *client; // this is a pointer to its parent client
        std::string buffer; 

        

        std::string method;
        std::string url;
        std::string version;
        std::multimap<std::string, std::string > headers;

        std::stringstream ss;
        std::ofstream body;
        std::string bodyString;

        int 		contentLength;
        std::string transferEncoding;

        std::string contentType;
        std::string boundary;
        
        std::string host;
        std::string connection;

        //** methods
        std::string	checkType(std::string path);
        void ParseFirstLine(std::string &line);
        void ParseHeaders(std::string &line);
        void ParseBody();
        void ParseChunkedBody();
        void ParseMultiPartBody();

		std::string	query;

        // void ParseBodyChunked();
		// int handle error();
};

// enum responseStat
// {
//     START,
//     FIRSTLINE,
//     HEADERS,
//     DONE
// };

class Response
{
		public:
			int			client_fd;
			Client		*client;
			Http		*http;
			Location	*location;
            bool 		GENERATE_RES;
			std::map<std::string, std::string>	contentTypes;	

			std::string		responseStr;
			std::string		body;
	
			Response();
            // Response(const Response &response);
			~Response();

			size_t	readPos = 0;
			int		responseSent = 0;
			size_t	sendPos = 0;
			int		started = 0;

			size_t	contentLength;

			std::ofstream	file1;

			int	step = 0;

			
			void	handleNormalReq();
			void	storeMimeTypes();
			void	sendChunked(std::ifstream &file);

			std::string	generateError(std::string error, int flag);

			int	checkAccess(std::string path);

			int		checkError(int	error);

//-------new methods----------------
			int	sendFlag = 0;
			std::ifstream	fileRead;
			std::ofstream	fileWrite;
			std::ofstream 	writeMultipart;
			int		checkReturn();
			void	parseUrl();
			void	getPath();
			int		checkResourseType();
			void	handleFile();
			void	handleDirectory();
			std::string	newPath;
			void	handleGet(int type);
			void	sendFile();
			void	sendDirectory();
			void	writeResponse();
			std::string		getIndex();
			void			handleNormalBody();
//----------------------------------

			void					getQuery();
			void					checkAllowedMethods();
			void					matchLocation(std::vector<Location> locations);
			void					checkCgi();
			void					checkPath();
			void					listDirectory();
			std::vector<Location>	getLocations(std::vector<Location> locations);

			void	listDirectory(std::string	newPath, DIR *dir);

			void	handleDelete();
			void	handlePost();
			void	handleMultipart();
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
        Session            session;

		int				cgiFlag;

        Cgi *cgi;
        // Response *response;
        // SocketWrapper *socket;
        Client();
        // Client(const Client &client);
        ~Client();
		std::string		path;
		Client(SocketWrapper *socket);


        //**  methods
        void handleCookies();
    	void handleRequest();
    	void cgi_handler();
    	// void generateResponse();
    	// void writeResponse();
    	// void checkInactivity();
		void	selectServer();
    	void	generateResponse();
        time_t lastActivity;
        
    	// void writeResponse();
    	// void checkInactivity();

};

std::string GetFutureTime();
std::string generateSessionId(size_t length) ;