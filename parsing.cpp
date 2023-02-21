#include "parser/parser.hpp"

Lexer *Parser::ptr = nullptr;
Http *Parser::http = nullptr;

int main()
{
    int pos = 0;
    Parser::lex("nginx.conf");
    Parser::parse();

    // print the content of the http direcives
    // for (auto i : Parser::getHttp()->servers[1].server_directives)
    // {
    //     std::cout << i.first << " : ";
    //     for (auto j : i.second)
    //         std::cout << j << " ";
    //     std::cout << std::endl;
    // }
    // for (auto i : Parser::lex()->lines)
    // std::cout << "line :" << i << std::endl;
    // std::string input;
    // while (input != "EOF")
    // input = Parser::lex()->next_token(Parser::lex()->input);
    // for (auto i : Parser::lex()->tokens)
    //     std::cout << i << std::endl;
    // std::vector<std::string> lines = Parser::lex()->lines;
    // std::cout << Parser::lex()->lines[2] << std::endl;

    // for(auto line : lines)
    // {
    //     Parser::lex()->set_input(line);
    //     std::string token = Parser::lex()->next_token();
    //     while (token != "EOF")
    //     {
    //         std::cout << token << std::endl;
    //         token = Parser::lex()->next_token();
    //     }

    // }

    // std::cout << Parser::lex()->next_token(lines[2]) << std::endl;
    // std::cout << Parser::lex()->next_token(lines[2]) << std::endl;

    return 0;
}





