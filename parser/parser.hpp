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

class Server;
class Location;
class Http
{
public:
    // TODO : make it multimap to store multiple values for the same key

    std::map<std::string, std::vector<std::string> > http_directives;
    std::vector<Server> servers;
};

class Server : public Http
{
public:
    // TODO : make it multimap to store multiple values for the same key

    std::map<std::string, std::vector<std::string> > server_directives;

    std::vector<Location> locations;
};

class Location : public Server
{
public:
    // TODO : make it multimap to store multiple values for the same key
    
    std::map<std::string, std::vector<std::string> > location_directives;
    std::vector<Location> locations;
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

    std::istringstream input_stream;

    int pos;
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
