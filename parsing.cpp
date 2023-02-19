#include "parser/parser.hpp"

Lexer *Parser::ptr = nullptr;

int main()
{
    int pos = 0;
    Parser::lex("nginx.conf");
    // for (auto i : Parser::lex()->lines)
    // std::cout << "line :" << i << std::endl;
    std::string input;
    // while (input != "EOF")
    // input = Parser::lex()->next_token(Parser::lex()->input);
    // for (auto i : Parser::lex()->tokens)
    //     std::cout << i << std::endl;
    std::vector<std::string> lines = Parser::lex()->lines;
    // std::cout << Parser::lex()->lines[2] << std::endl;

    std::cout << "line : " << lines[2] << std::endl;

    std::cout << Parser::lex()->next_token(lines[2]) << std::endl;
    std::cout << Parser::lex()->next_token(lines[2]) << std::endl;
    std::cout << Parser::lex()->next_token(lines[2]) << std::endl;
    std::cout << Parser::lex()->next_token(lines[2]) << std::endl;
    // std::cout << Parser::lex()->next_token(lines[2]) << std::endl;
    // std::cout << Parser::lex()->next_token(lines[2]) << std::endl;

    return 0;
}
