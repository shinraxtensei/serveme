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

Http *Parser::getHttp()
{
    if (Parser::http == NULL)
        Parser::http = new Http();
    return http;
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
        if (line.size() > 0)
            lines.push_back(line + '\n');
    }
    this->input = input;
};

void Lexer::print_input()
{
    std::cout << this->input << std::endl;
}

void Lexer::set_input(const std::string &input)
{
    this->input_stream.clear();
    this->input_stream.str(input);
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

std::string Lexer::next_token(bool consume)
{
    // TODO : when there is ; after the quote, it will be considered as part of the token and will be added to the token
    // TODO : needs to be able to save the position in case we just want to match the token and not consume it
    std::streampos pos = input_stream.tellg();
    skip_whitespace(input_stream);
    std::string token;
    char c;

    if (input_stream.eof())
    {
        return "EOF";
    }

    c = input_stream.get();
    if (c == '"' || c == '\'')
    {
        char quote = c;
        while (input_stream && (c = input_stream.get()) != quote)
        {
            token += c;
        }
        token = quote + token + quote;
        while (input_stream && !is_whitespace(input_stream.peek()))
            token += input_stream.get();
    }
    else
    {
        while (input_stream && !is_whitespace(c))
        {
            token += c;
            c = input_stream.get();
        }
    }

    if (consume == false)
        input_stream.seekg(pos);
    this->tokens.push_back(token);
    return token;
}

bool Parser::match(std::string token)
{
    if (token == Parser::lex()->next_token(false))
    {
        Parser::lex()->next_token(true);
        return true;
    }
    return false;
}

void Parser::parse_directives(int type)
{
    // std::cout <<" parse_directives : " << type << std::endl;
    std::string directive = Parser::lex()->next_token(true);
    std::vector<std::string> values;
    std::string value;
    while (1)
    {
        value = Parser::lex()->next_token(true);
        if (value.back() == ';')
        {
            value.pop_back();
            values.push_back(value);
            break;
        }
        values.push_back(value);
        // std::cout << Parser::lex()->next_token() << std::endl;
    }
    std::cout << directive + " : ";
    for (int i = 0; i < values.size(); i++)
        std::cout << values[i] << " ";
    std::cout << std::endl;
}

void Parser::parse_location()
{
    // TODO : while the token is not {  it will be considered as part of the location

    std::cout << "location : ";

    while (!Parser::match("{"))
        std::cout << Parser::lex()->next_token(true) << " ";
    std::cout << std::endl
              << "{" << std::endl;
    while (1)
    {
        if (Parser::match("location"))
            Parser::parse_location();
        else if (Parser::match("}"))
        {
            std::cout << "}\n";
            return;
        }
        else
            Parser::parse_directives(2);
    }
}

void Parser::parse_server()
{

    std::cout << "server" << std::endl;
    if (Parser::match("{"))
    {
        std::cout << '{' << std::endl;

        while (1)
        {
            if (Parser::match("location"))
                parse_location();
            else if (Parser::match("}"))
            {
                std::cout << "}\n";
                break;
            }
            else
                parse_directives(1);
        }
    }
}

void Parser::parse()
{
    // TODO : need to parse the location bloc better (take tokens until the '{')
    // TODO : need to store the data in the appropriate data structure
    // TODO : need to handle errors , types , etc
    // TODO : implement grammar rules precisely

    Parser::lex()->set_input(Parser::lex()->input);

    if (Parser::match("http"))
    {
        std::cout << "http\n";
        if (Parser::match("{"))
        {
            std::cout << "{\n";
            while (1)
            {
                if (Parser::match("server"))
                    parse_server();
                else if (Parser::match("}"))
                {
                    std::cout << "}\n";
                    break;
                    ;
                }
                else
                    parse_directives(0);
            }
        }
    }
}
