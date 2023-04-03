#pragma once
// #include "parser.hpp"
#include "servme.hpp"



class Lexer
{
public:
    Lexer(){};
    Lexer(std::string input);
    ~Lexer(){};
    void set_input(const std::string &input);
    void print_input();
    std::string next_line();
    std::string next_token(bool consume);
    bool errors_check();
    std::vector<std::string> tokens;
    std::vector<std::string> lines;
    std::vector<std::string> all_directs;
    std::istringstream input_stream;
    std::string input;
    
    int streamPos;
    void refrechPos();


};