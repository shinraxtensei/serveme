#include "../inc/core.hpp"

Request::Request()
{
    this->state = Stat::START;
    this->bodyType = BodyType::NONE;
    this->host = "";
    this->connection = "";
    this->contentLength = 0;
    this->transferEncoding = "";
    this->method = "";
    this->url = "";
    this->version = "";
    this->bodyString = "";
}

Request &Request::operator=(const Request &other)
{
    this->state = other.state;
    this->core = other.core;
    this->client = other.client;
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



std::vector<std::string> getStringTokens(std::string const &str)
{
    Parser::lex()->set_input(str);
    std::vector<std::string> tokens;
    while(true)
    {
        std::string token = Parser::lex()->next_token(true);
        if (token == "EOF")
            break;
        tokens.push_back(token);
    }
    return tokens;
}



void Request::ParseFirstLine(std::string &line)
{
    std::cout << CYAN << "STATE: " << (this->state == FIRSTLINE ? "FIRSTLINE" : "weird") << RESET << std::endl;


    // std::cout << "last char: " << line.back() << std::endl;
    
    std::vector<std::string> knownMethods;
    knownMethods = getStringTokens("GET HEAD POST PUT DELETE CONNECT OPTIONS TRACE");

    Parser::lex()->set_input(line);

    this->method = Parser::lex()->next_token(true);
    this->url = Parser::lex()->next_token(true);
    this->version = Parser::lex()->next_token(true);

    // if (Parser::lex()->next_token(true) != "EOF")
    //     std::cout << "Error: Invalid request line." << std::endl;
        // throw std::runtime_error("Error: Invalid request line.");

    if (std::find(knownMethods.begin(), knownMethods.end(), this->method) == knownMethods.end())
        std::cout << "Error: Invalid method." << std::endl;
    if (this->url.size() > 2048 || checkValidChars(this->url) == 0)
        std::cout << "Error: Invalid URL." << std::endl;
        // throw std::runtime_error("Error: Invalid URL.");
    // if (this->version != "HTTP/1.1")
    if (this->version.find("HTTP/1.1") == std::string::npos && this->version.find("HTTP/1.0") == std::string::npos)
        std::cout << "Error: Invalid HTTP version." << std::endl;
        // throw std::runtime_error("Error: Invalid HTTP version.");
}

void Request::ParseHeaders(std::string &line)
{
    std::cout << CYAN << "STATE: " << (this->state == HEADERS ? "HEADERS" : "weird") << RESET << std::endl;

    if (line == "\r\n" || line == "\n")
        return;

    std::pair<std::string, std::vector<std::string>> pair;
    std::string key;
    std::string value;
    Parser::lex()->set_input(line);

    key = Parser::lex()->next_token(true);
    value = &line[key.size() + 1];

    if (key.back() != ':')
        std::cout << "Error: Invalid header line." << std::endl;

    if (key == "host:")
    {
        this->host = value;
        this->client = &Servme::getCore()->map_clients[this->client_fd]; //TODO: change this to be in the constructor 
        this->client->selectServer();

    }


    if (key == "content-length:")
    {
        this->bodyType = BodyType::NONE;
        Parser::lex()->set_input(value);
        if (Parser::lex()->next_token(false).find_first_not_of("0123456789") != std::string::npos)
            std::cout << "Error: Invalid content-length." << std::endl;
        value = Parser::lex()->next_token(false);
        this->contentLength = atoi(value.c_str());
        if (this->contentLength > this->client->server->client_max_body_size)
            throw std::runtime_error("Error: Content-Length is too big.");
    }

    if (key == "transfer-encoding:")
    {
        if (value.find("chunked") != std::string::npos)
        {
            this->bodyType = BodyType::CHUNKED;
        }
        
        // TODO : if it has chunked, then it should be chunked
        this->transferEncoding = value;
    }
    if (key == "content-type:")
    {
        // TODO : if it has multipart, then it should be multipart
        if (value.find("multipart") != std::string::npos)
            this->bodyType = BodyType::MULTIPART;       
    }

    if (key == "connection:")
        this->connection = value;

    this->headers.insert(std::make_pair(key, value));

}

void Request::ParseBody()
{
    if (this->state == Stat::END)
        return;
    std::cout << CYAN << "STATE: " << (this->state == BODY ? "BODY normal" : "weird") << RESET << std::endl;
    if (this->state == Stat::END)
        return;
    static int bodySize = 0;
    char buffer[1024];

    int bytesRead = read(this->client_fd, buffer, std::min((this->contentLength - bodySize), 1024));
    if (bytesRead == -1)
        throw std::runtime_error("Error: read() failed.");
    if (bytesRead == 0)
    {
        this->state = Stat::END;
        return;
    }
        // throw std::runtime_error("Error: read() returned 0.");
    bodySize += bytesRead;
    this->bodyString += std::string(buffer, bytesRead);
    if ((int)this->bodyString.size() == this->contentLength)
        this->state = Stat::END;

    std::cout << this->bodyString << std::endl;
}




// TODO :  remake this shit
void Request::ParseChunkedBody()
{
    std::cout << CYAN << "STATE: " << (this->state == BODY ? "BODY chunked" : "weird") << RESET << std::endl;
    if (this->state == Stat::END)
        return;
    std::string stringSize;
    if (this->state & Stat::CHUNKED_SIZE)
    {
        std::cout << "CHUNKED SIZE" << std::endl;
       
        int bytesRead = read(this->client_fd, &stringSize, 1024);
        if (bytesRead == -1)
            throw std::runtime_error("Error: read() failed.");
        if (bytesRead == 0)
            throw std::runtime_error("Error: read() returned 0.");
        if (stringSize == "\r\n" || stringSize == "\n")        
            return;

        if (stringSize.find("\r\n") != std::string::npos)
            stringSize = stringSize.substr(0, stringSize.find("\r\n"));
        else if (stringSize.find("\n") != std::string::npos)
            stringSize = stringSize.substr(0, stringSize.find("\n"));

        if (stringSize.find_first_not_of("0123456789") != std::string::npos)
            throw std::runtime_error("Error: Invalid chunk size.");

        std::cout << "stringSize: " << stringSize << std::endl;
        this->state = Stat::CHUNKED_DATA;
    }
    if (this->state & Stat::CHUNKED_DATA)
    {
        std::cout << "CHUNKED DATA" << std::endl;
        char buffer[atoi(stringSize.c_str())];
        int bytesRead = read(this->client_fd, buffer, atoi(stringSize.c_str()));
        if (bytesRead == -1)
            throw std::runtime_error("Error: read() failed.");
        if (bytesRead == 0)
            throw std::runtime_error("Error: read() returned 0.");
        this->bodyString += std::string(buffer, bytesRead);
        this->state = Stat::CHUNKED_SIZE;
    }
    std::cout << this->bodyString << std::endl;
}

void Request::ParseMultiPartBody()
{

}