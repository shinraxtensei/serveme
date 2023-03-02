#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <set>
#include <unordered_set>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "core.hpp"
#include "socketWrapper.hpp"
#include <unordered_map>


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
    std::string index;
    std::string allowed_methos;
    std::string error_page;
};


class SocketWrapper;
class Server : public Http
{
public:
    Server();
    ~Server();
    std::map<std::string, std::vector<std::string> > server_directives;
    std::vector<Location> locations;

    std::vector<int> fds;

    // **** mandatory directives ****
    SocketWrapper *sock; //* this is a socket wrapper 
    // SocketWrapper *Socket();
    // int sockfd;

	std::unordered_map<std::string, std::vector<std::string> >	request;
    void HandleRequest(int fd);
    void connect();
    void HandleResponse();

    int listen; 


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
    std::string Return ;
};

class Lexer
{
public:
    Lexer(){};
    Lexer(std::string input);
    ~Lexer(){};
    void set_input(const std::string &input);
    void print_input();
    std::string next_token(bool consume);
    bool errors_check();
    std::vector<std::string> tokens;
    std::vector<std::string> lines;
    std::vector<std::string> all_directs;
    std::istringstream input_stream;

    std::string input;
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
    static void parse_location();
};
