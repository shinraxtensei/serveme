#include "../inc/core.hpp"

Request::Request()
{
    this->state = FIRSTLINE;
    this->host = "";
    this->connection = "";
    this->contentLength = 0;
    this->transferEncoding = "";
    this->method = "";
    this->url = "";
    this->version = "";

}

Request &Request::operator=(const Request &other)
{
    this->state = other.state;
    this->core = other.core;
    this->client = other.client;
    this->server = other.server;
    this->buffer = other.buffer;
    // this->ss = other.ss;
    this->headers = other.headers;
    // this->body = other.body;
    this->method = other.method;
    this->url = other.url;
    this->version = other.version;
    this->contentLength = other.contentLength;
    this->transferEncoding = other.transferEncoding;
    this->host = other.host;
    this->connection = other.connection;
    return *this;
}

Request::Request(const Request &other)
{
    *this = other;
}


Request::~Request()
{
}




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
    for (it = this->core->get_http()->servers.begin(); it != this->core->get_http()->servers.end(); it++)
    {
        if (it->ipPort.second == this->client->socket->get_listenPair().second)
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
    std::cout << CYAN << "STATE: " << (this->state == FIRSTLINE ? "FIRSTLINE" : "weird") << RESET << std::endl;
    std::string knownMethods = "GET HEAD POST PUT DELETE CONNECT OPTIONS TRACE";

    Parser::lex()->set_input(line);

    this->method = Parser::lex()->next_token(true);
    this->url = Parser::lex()->next_token(true);
    this->version = Parser::lex()->next_token(true);
    if (Parser::lex()->next_token(true) != "EOF")
        std::cout << "Error: Invalid request line." << std::endl;
        // throw std::runtime_error("Error: Invalid request line.");

    if (knownMethods.find(this->method) == std::string::npos)
        std::cout << "Error: Invalid method." << std::endl;
        // throw std::runtime_error("Error: Invalid method.");
    if (this->url.size() > 2048 || checkValidChars(this->url) == 0)
        std::cout << "Error: Invalid URL." << std::endl;
        // throw std::runtime_error("Error: Invalid URL.");
    if (this->version != "HTTP/1.1")
        std::cout << "Error: Invalid HTTP version." << std::endl;
        // throw std::runtime_error("Error: Invalid HTTP version.");

    this->state = HEADERS;
}

void Request::ParseHeaders(std::string &line)
{
    std::cout << CYAN << "STATE: " << (this->state == HEADERS ? "HEADERS" : "weird") << RESET << std::endl;

    std::pair<std::string, std::vector<std::string>> pair;
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
        std::cout << "Error: Invalid header line." << std::endl;

    // while (Parser::lex()->next_token(true) != "EOF")
    //     values.push_back(Parser::lex()->next_token(true));
    values.push_back(Parser::lex()->next_token(true));

    std::cout << "key: " << key << std::endl;
    std::cout << "values: " << values[0] << std::endl;

    if (key == "host:")
    {
        this->host = values[0];
        // this->selectServer();
    }
    if (key == "content-length:")
    {
        this->contentLength = std::stoi(values[0]);
        // if (this->contentLength > this->server->client_max_body_size)
        //     throw std::runtime_error("Error: Content-Length is too big.");
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
    std::cout << YELLOW << "method : " << RESET << this->method << std::endl;
    std::cout << YELLOW << "url : " << RESET << this->url << std::endl;
    std::cout << YELLOW << "version : " << RESET << this->version << std::endl;

    std::cout << YELLOW << "host : " << RESET << this->host << std::endl;
    std::cout << YELLOW << "contentLength : " << RESET << this->contentLength << std::endl;
    std::cout << YELLOW << "transferEncoding : " << RESET << this->transferEncoding << std::endl;
    std::cout << YELLOW << "connection : " << RESET << this->connection << std::endl;

    std::cout << CYAN << "STATE: " << (this->state == BODY ? "BODY" : "weird") << RESET << std::endl;

}
