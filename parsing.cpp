#include "parser/parser.hpp"

Lexer *Parser::ptr = nullptr;

int main()
{
    int pos = 0;
    Parser::lex("nginx.conf");
    std::string input;
    while (input != "EOF")
        input = Parser::lex()->next_token();
    for (auto i : Parser::lex()->tokens)
        std::cout << i << std::endl;
    return 0;
}
