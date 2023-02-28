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
	
	std::unordered_map<std::string, std::vector<std::string> >	request;
	std::string		request_body;

    // **** mandatory directives ****
    SocketWrapper *sock; //* this is a socket wrapper 
    // SocketWrapper *Socket();
    // int sockfd;

    void HandleRequest(int fd);
    void connect();
    void HandleResponse();

    int listen; 

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
