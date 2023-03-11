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



class Server;
class Location;



class Http
{
public:
    std::map<std::string, std::vector<std::string> > http_directives;
    std::vector<Server> servers;
    
    // **** mandatory directives ****
    
    std::string root;
    std::vector<std::string> index;
    std::vector<std::string> allowed_methods;
    std::vector<std::string> error_page;
    bool autoindex;
    int client_max_body_size;
};


class SocketWrapper;

class Server : public Http
{
public:
    Server(){};
    ~Server(){};
    std::map<std::string, std::vector<std::string> > server_directives;
    std::vector<Location> locations;

    std::vector<int> fds;
	
	std::unordered_map<std::string, std::vector<std::string> >	request;
	std::string		request_body;

    // **** mandatory directives ****
    SocketWrapper *sock; //* this is a socket wrapper 
    // SocketWrapper *Socket();
    // int sockfd;

    // void HandleRequest(int fd);
    // void connect();
    // void HandleResponse();


    // **** optional directives ****
    std::pair<std::string ,int> ipPort;
    std::string server_name;



};

class Location : public Server
{
public:
    std::map<std::string, std::vector<std::string> > location_directives;
    std::vector<Location> locations;
      // **** mandatory directives ****  
    // std::string Return ;
    std::pair<int ,std::string > Return;
};





class Lexer;
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
    static void parse_location();
    static void init_servers();
};
