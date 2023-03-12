#include "../inc/core.hpp"


int checkValidChars(std::string &str)
{
    std::string valid_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";
    for (size_t i = 0; i < str.size(); i++)
    {
        if (valid_chars.find(str[i]) == std::string::npos)
            return 0;
    }
    return 1;
}

void Request::selectServer()
{
    // TODO  :select the server which has the same port and ip from where the request came from, and the same server_name as the host header
    // std::vector<Server>::iterator it;
    // for (it  = this->core->get_http()->servers.begin(); it != this->core->get_http()->servers.end(); it++)
    // {
    // }

}




void Request::ParseFirstLine(std::string &line)
{
    std::string knownMethods = "GET HEAD POST PUT DELETE CONNECT OPTIONS TRACE";

    Parser::lex()->set_input(line);
    this->method = Parser::lex()->next_token(true);
    this->url = Parser::lex()->next_token(true);
    this->version = Parser::lex()->next_token(true);
    if (Parser::lex()->next_token(true) != "EOF")
        throw std::runtime_error("Error: Invalid request line.");


    if (knownMethods.find(this->method) == std::string::npos)
        throw std::runtime_error("Error: Invalid method.");
    if (this->url.size() > 2048 ||  checkValidChars(this->url) == 0)
        throw std::runtime_error("Error: Invalid URL.");
    if (this->version != "HTTP/1.1")
        throw std::runtime_error("Error: Invalid HTTP version.");


    this->state = HEADERS;
}

void Request::ParseHeaders(std::string &line)
{
    std::pair<std::string , std::vector<std::string> > pair;
    std::string key;
    std::vector<std::string> values;
    Parser::lex()->set_input(line);

    key = Parser::lex()->next_token(true);

    if (key.back() != ':')
        throw std::runtime_error("Error: Invalid header line.");

    while (Parser::lex()->next_token(true) != "EOF")
        values.push_back(Parser::lex()->next_token(true));

    if (key == "host:")
    {
        this->host = values[0];
        this->selectServer();
    }



    pair = std::make_pair(key, values);


}


void Request::ParseBody()
{

}


