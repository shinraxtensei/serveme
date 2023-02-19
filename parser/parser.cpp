#include "parser.hpp"

Lexer *Parser::lex(std::string filename)
{
    if (Parser::ptr == NULL)
        Parser::ptr = new Lexer(filename);
    return ptr;
}
Lexer *Parser::lex()
{
    if (Parser::ptr == NULL)
    {
        std::cout << "Error: No file specified" << std::endl;
        return NULL;
    }
    return ptr;
}

Lexer::Lexer(std::string filename) : pos(0)
{
    std::ifstream file(filename);
    std::string input;
    std::string line;
    int pos = 0;
    while (std::getline(file, line))
    {

        input += line;
        input += '\n';
        if (line.size() > 0 )
            lines.push_back(line + '\n');
    }
    this->input = input;
};

void Lexer::print_input()
{
    std::cout << this->input << std::endl;
}

static bool is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n';
}

static void skip_whitespace(std::istringstream &input)
{
    while (input)
    {
        char c = input.peek();
        if (is_whitespace(c))
            input.get();
        else if (c == '#')
            while (input && input.get() != '\n')
                ;
        else
            break;
    }
}

std::string Lexer::next_token(std::string input)
{
    static std::istringstream input_stream(input);
    skip_whitespace(input_stream);
    std::string token;
    char c;

    if (input_stream.eof())
        return "EOF";

    c = input_stream.get();
    if (c == '"' || c == '\'')
    {
        char quote = c;
        while (input_stream && (c = input_stream.get()) != quote)
        {
            token += c;
        }
        token = quote + token + quote;
    }
    else
    {
        while (input_stream && !is_whitespace(c))
        {
            token += c;
            c = input_stream.get();
        }
    }

    this->tokens.push_back(token);
    return token;
}