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
    std::vector<Server>::iterator it;
    std::vector<Server> candidates;
    for (it  = this->core->get_http()->servers.begin(); it != this->core->get_http()->servers.end(); it++)
    {
        if (it->ipPort.second == this->client->socket->get_listenPair().second && it->ipPort.first == this->client->socket->get_listenPair().first)
            candidates.push_back(*it);
    }
    if (candidates.size() == 0)
        throw std::runtime_error("Error: No server found for this request.");
    else
    {
        for (it = candidates.begin(); it != candidates.end(); it++)
        {
            if (it->server_name == this->host)
            {
                this->server = &(*it);
                break;
            }
            else
                this->server = &candidates[0];
        }
    }

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


    if (line.find("\r\n") != std::string::npos)
    {
        this->state = BODY;
        return;
    }

    if (key.back() != ':')
        throw std::runtime_error("Error: Invalid header line.");

    while (Parser::lex()->next_token(true) != "EOF")
        values.push_back(Parser::lex()->next_token(true));



    if (key == "host:")
    {
        this->host = values[0];
        this->selectServer();
    }
    if (key == "content-length:")
    {
        this->contentLength = std::stoi(values[0]);
        if (this->contentLength > this->server->client_max_body_size)
            throw std::runtime_error("Error: Content-Length is too big.");
    }
    if (key == "transfer-encoding:")
        this->transferEncoding = values[0];
    
    if (key == "connection:")
        this->connection = values[0];

    

    pair = std::make_pair(key, values);
    this->headers.insert(pair);

}


void Request::ParseBody()
{

}


