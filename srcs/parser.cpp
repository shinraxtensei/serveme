#include "../inc/parser.hpp"
#include "../inc/core.hpp"


//  this is a singleton for the lexer object
Lexer *Parser::lex(std::string filename)
{
    if (Parser::ptr == NULL)
        Parser::ptr = new Lexer(filename);
    return ptr;
}
//  this is a singleton for the lexer object
Lexer *Parser::lex()
{
    if (Parser::ptr == NULL)
    {
        std::cout << "Error: No file specified" << std::endl;
        return NULL;
    }
    return ptr;
}

// this is a singleton for the http object
Http *Parser::getHttp()
{
    if (Parser::http == NULL)
        Parser::http = new Http();
    return http;
}



bool Lexer::errors_check()
{
    std::stack<std::string> stack;
    for (size_t i = 0; i < this->lines.size(); i++)
    {
        int size = 0;
        std::string newline;
        std::vector<std::string> tokens;
        this->set_input(this->lines[i]);
        while (this->next_token(false) != "EOF")
        {
            std::string token;
            token = this->next_token(true);

            // * the for unclosed curly braces

            if (token == "{" || token == "'" || token == "\"")
                stack.push(token);
            else if (token == "}" || token == "'" || token == "\"")
            {
                if (stack.empty())
                    stack.push(token);
                else
                    stack.pop();
            }
            //*******************************


            newline += token + " ";
            tokens.push_back(token);
            size += token.size();
        }
        if (!tokens.empty() && size > 0 && tokens.back().back() != ';' && (tokens[0] != "server" && tokens[0] != "location" && tokens[0] != "http" && tokens[0] != "events" && tokens[0] != "{" && tokens[0] != "}" && tokens.back().back() != '\'' && tokens.back().back() != '"'))
        {
            std::cout << tokens[0] << std::endl;
            std::cout << "Error: missing ; at the end of the line" << std::endl;
            // return false;
            // TODO : throw an exception here
            exit(1);
        }
    }
    if (!stack.empty())
    {
        std::cout << "Error: unclosed curly braces" << std::endl;
        // return false;
        // TODO : throw an exception here
        exit(1);
    }

    // TODO :  check if ; is at the end of the line if its not any special token
    // TODO :  check the first directive is not a number ... directives should be known

    return true;
}

// this is the constructor for the lexer

Lexer::Lexer(std::string filename)
{   
    // *** this is for inserting all the directives inside a vector in order to check if the directive is valid or not

    std::ifstream dir("all_directs");
    std::string input;
    std::string line;
    while (std::getline(dir, line))
        this->all_directs.push_back(line);

    // *********************************************************



    std::ifstream file(filename);
    while (std::getline(file, line))
    {

        input += line;
        input += '\n';
        if (line.size() > 0)
            lines.push_back(line + '\n');
    }
    if (this->errors_check())
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

    if (input_stream.eof())
        return "EOF";
    while (!input_stream.eof() && !is_whitespace(input_stream.peek()))
    {
        if (input_stream.peek() == '"' || input_stream.peek() == '\'' || input_stream.peek() == '(')
        {
            char quote = input_stream.get();
            token += quote;
            while (!input_stream.eof() && (input_stream.peek() != quote) && (quote == '(' && input_stream.peek() != ')'))
                token += input_stream.get();
        }
        if (is_whitespace(input_stream.peek()))
            break;
        token += input_stream.get();
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
    }

    //* a pair or key and values
    std::pair<std::string, std::vector<std::string>> pair(directive, values);
    
    //* this is the pair of (iterator , bool ) to check if the pair is inserted or not , if not then its already exists
    std::pair<std::map<std::string, std::vector<std::string>>::iterator, bool> ret;

    if ( find(lex()->all_directs.begin(),lex()->all_directs.end(), directive) == lex()->all_directs.end())
    {
        std::cout << directive << std::endl;
        std::cout << "Error: directive not found" << std::endl;
        exit(1);
    }


    if (type == 0)
    {
        ret = Parser::getHttp()->http_directives.insert(pair);
        if (ret.second == false)
        {
            std::cout << "Error: directive already exists" << std::endl;
            exit(1);
        }
    }

    else if (type == 1)
    {
        ret = Parser::getHttp()->servers.back().server_directives.insert(pair);
        if (ret.second == false)
        {
            std::cout << "Error: directive already exists" << std::endl;
            exit(1);
        }
    }
    else if (2)
    {
        ret = Parser::getHttp()->servers.back().locations.back().location_directives.insert(pair);
        if (ret.second == false)
        {
            std::cout << "Error: directive already exists" << std::endl;
            exit(1);
        }
    }
    else
        std::cout << "Error: type not found" << std::endl;
}

void Parser::parse_location()
{
    //  TODO : while the token is not {  it will be considered as part of the location
    Parser::getHttp()->servers.back().locations.push_back(Location());
    while (1)
    {
        if (Parser::match("location"))
            Parser::parse_location();
        else if (Parser::match("}"))
            return;
        else
            Parser::parse_directives(2);
    }
}

void Parser::parse_server()
{
    Parser::getHttp()->servers.push_back(Server());
    if (Parser::match("{"))
    {
        while (1)
        {
            if (Parser::match("location"))
                parse_location();
            else if (Parser::match("}"))
                break;
            else
                parse_directives(1);
        }
    }
}

void Parser::parse()
{
    // TODO: need to store the server name , and patse the listen directive to look for the port number and the ip address
    // TODO : store the ports in a set or a map , it should be linked to its servers


    Parser::lex()->set_input(Parser::lex()->input);

    while (Parser::lex()->next_token(false) != "EOF")
    {
        if (Parser::match("http"))
        {
            if (Parser::match("{"))
            {
                while (1)
                {
                    if (Parser::match("server"))
                        parse_server();
                    else if (Parser::match("}"))
                        break;
                    else
                        parse_directives(0);
                }
            }
        }
        else
            parse_directives(0);
    }
}