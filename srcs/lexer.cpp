#include "../inc/lexer.hpp"



//  this is a singleton for the lexer object
Lexer *Parser::lex(std::string filename)
{
    if (Parser::ptr == nullptr)
        Parser::ptr = new Lexer(filename);
    return ptr;
}
//  this is a singleton for the lexer object
Lexer *Parser::lex()
{
    if (Parser::ptr == nullptr)
    {
        std::cout << "Error: No file specified" << std::endl;
        return NULL;
    }
    return ptr;
}

// this is a singleton for the http object
Http *Parser::getHttp()
{
    if (Parser::http == nullptr)
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
            throw std::runtime_error("Error: missing ; at the end of the line: \n" + newline);

    }
    if (!stack.empty())
        throw std::runtime_error("Error: unclosed curly braces");


    return true;
}

// this is the constructor for the lexer

Lexer::Lexer(std::string filename)
{   
    // *** this is for inserting all the directives inside a vector in order to check if the directive is valid or not
    this->streamPos = 0;

    std::ifstream dir("all_directs");
    std::string input;
    std::string line;
    while (std::getline(dir, line))
        this->all_directs.push_back(line);
    

    // *********************************************************



    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Error: file not found");
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
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' ;
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


void Lexer::refrechPos()
{
    this->input_stream.seekg(this->streamPos);
    this->next_token(true);
}


std::string Lexer::next_line()
{
    skip_whitespace(input_stream);
    std::string line;
    if (input_stream.eof())
        return "EOF";
    while(!input_stream.eof())
    {
        if (input_stream.peek() == '\n')
        {
            input_stream.get();
            break;
        }
        else  if (input_stream.peek() == '\r')
        {
            input_stream.get();
            input_stream.get();
            break;
        }
        line += input_stream.get();
    }
    return line;
}




std::string Lexer::next_token(bool consume)
{
    std::streampos pos = input_stream.tellg();
    this->streamPos = this->input_stream.tellg();

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
            {
                token += input_stream.get();

            }
        }
        if (is_whitespace(input_stream.peek()))
            break;
        token += input_stream.get();
    }
    char last = token.back();
    if (last == -1)
        token.pop_back();

    token.erase(std::remove(token.begin(), token.end(), '\r'), token.end());
    token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());
    if (consume == false)
        input_stream.seekg(pos);
    


    this->tokens.push_back(token);
    return token;
}