#pragma once

#include "servme.hpp"

// #include "socketWrapper.hpp"
// #include "core.hpp"
// #include "lexer.hpp"

#define BLUE "\033[0;34m"
#define RESET "\033[0m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define MAGENTA "\033[0;35m"
#define CYAN "\033[0;36m"
#define BOLDBLACK "\033[1;30m"
#define BOLDRED "\033[1;31m"
#define BOLDGREEN "\033[1;32m"
#define BOLDYELLOW "\033[1;33m"
#define BOLDBLUE "\033[1;34m"


class Lexer;
class Server;
class Location;
class SocketWrapper;

class Http
{
	
	public:
		Http();
		~Http();
		std::map<std::string, std::vector<std::string> > http_directives;
		std::vector<Server> servers;

		// **** mandatory directives ****

		std::string root;
		std::vector<std::string> index;
		std::vector<std::string> allowed_methods;
		// std::vector<std::string> error_page;
		std::pair<int, std::vector<std::string> > error_page;
		bool autoindex;
		int client_max_body_size;
};

class Server : public Http
{
	public:
		Server();
		~Server();

		std::map<std::string, std::vector<std::string> > server_directives;
		std::vector<Location> locations;
		std::vector<int> fds;
		std::unordered_map<std::string, std::vector<std::string> > request;
		std::string request_body;

		// **** mandatory directives ****
		SocketWrapper *sock; //* this is a socket wrapper
						 // SocketWrapper *Socket();
						 // int sockfd;

		// **** optional directives ****
		std::pair<std::string, int>	ipPort;
		std::string					server_name;
};

class Location : public Server
{
	public:

		Location();
		~Location();
		std::map<std::string, std::vector<std::string> > location_directives;
		std::vector<Location> locations;
		// **** mandatory directives ****
		// std::string Return ;
		std::pair<int, std::string> Return;

		// return syntax [return url type]
		// allowed type = permanently / temporary
		int	returned;
		std::string	returnUrl;
		std::string returnType;

		std::string path;
		// std::string root;

};

class Parser
{
	private:
		static Lexer *ptr;
		static Http *http;

	public:
		static Lexer *lex(std::string filename);
		static Lexer *lex();
		static Http *getHttp();
		static void parse();
		static bool match(std::string token);
		static void parse_directives(int type);
		static void parse_server();
		static void parse_location(int sublocation);
		static void init_http();
		static void init_servers();
		static void init_locations(int index);
};
