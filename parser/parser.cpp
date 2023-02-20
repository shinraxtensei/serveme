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
        if (line.size() > 0 )
            lines.push_back(line + '\n');
    }
    this->input = input;
};

void Lexer::print_input()
{
    std::cout << this->input << std::endl;
}

void Lexer::set_input(const std::string& input) {
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

    std::string Lexer::next_token() {
        skip_whitespace(input_stream);
        std::string token;
        char c;

        if (input_stream.eof()) {
            return "EOF";
        }

        c = input_stream.get();
        if (c == '"' || c == '\'') {
            char quote = c;
            while (input_stream && (c = input_stream.get()) != quote) {
                token += c;
            }
            token = quote + token + quote;
        }
        else {
            while (input_stream && !is_whitespace(c)) {
                token += c;
                c = input_stream.get();
            }
        }

        this->tokens.push_back(token);
        return token;
    }


bool Parser::match(std::string token)
{
    if (token == Parser::lex()->next_token())
        return true;
    return false;
}

void Parser::parse_directives(int type){
    std::string directive = Parser::lex()->next_token();
    std::vector<std::string> values;
    while (Parser::lex()->next_token().back() != ';')
    {
        values.push_back(Parser::lex()->next_token());
    }
    if (type == 0)
        Parser::getHttp()->http_directives[directive] = values;
    // else if (type == 1)
    // else if (type == 2)
    else
        std::cout << "Error: Invalid type" << std::endl;

}

void Parser::parse_location(){
    if (Parser::match("{"))
    {
        if (Parser::match("location"))
            parse_location();
        else if (Parser::match("}"))
            return;
        else
            parse_directives(2);
    }
}

void Parser::parse_server(){
    if (Parser::match("{"))
    {
        if (Parser::match("location"))
            parse_location();
        else if (Parser::match("}"))
            return;
        else
            parse_directives(1);
    }
}


void Parser::parse()
{
    for(int i = 0 ; i < Parser::lex()->lines.size() ; i++)
    {
        std::string line = Parser::lex()->lines[i];
        std::cout << line << std::endl;
        Parser::lex()->set_input(line);
        std::string token = Parser::lex()->next_token();
        if (Parser::match("http"))
        {
            if (Parser::match("{"))
            {
                if (Parser::match("server"))
                    parse_server();
                else if (Parser::match("}"))
                    return;
                else
                    parse_directives(0);
            }
        }
    }
}