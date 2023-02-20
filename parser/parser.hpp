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

// class location : public server
// {
//     public:
//         std::map<std::string , std::pair<std::string , std::string> > directives;
//         std::vector<location> locations;
// };

// class server : public http
// {
//     public:
//         std::map<std::string , std::pair<std::string , std::string> > directives;
//         std::vector<location> locations;
// };

// class http
// {
//     public:
//         std::map<std::string , std::pair<std::string , std::string> > directives;
//         std::vector<server> servers;

// };

class Lexer
{
public:
    Lexer(){};
    Lexer(std::string input);
    ~Lexer(){};
    void set_input(const std::string& input);
    void print_input();
    std::string next_token();
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

public:
    static Lexer *lex(std::string filename);
    static Lexer *lex();
};
