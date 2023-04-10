#include "../inc/core.hpp"


Multipart_ENV::Multipart_ENV()
{
    this->file_name = "";
    this->content_type = "";
    this->data = "";
        this->pos = 0;
}

Multipart_ENV::Multipart_ENV(std::string fileName , std::string contentType, std::string data)
{
    this->file_name = fileName;
    this->content_type = contentType;
    this->data = data;
        this->pos = 0;
}

Multipart_ENV::Multipart_ENV(std::string fileName , std::string contentType)
{
    this->file_name = fileName;
    this->content_type = contentType;
    this->data = "";
        this->pos = 0;

}




Multipart_ENV::~Multipart_ENV()
{

}

Request::Request()
{


    this->state = Stat::START;
    this->bodyType = BodyType::NONE;
    this->host = "";
    this->connection = "";
    this->contentLength = 1024;
    this->transferEncoding = "";
    this->method = "";
    this->url = "";
    this->version = "";
    this->bodyString = "";
    this->contentType = "";
    this->boundary = "";
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

    this->client = &Servme::getCore()->map_clients[this->client_fd];
    
    this->client->selectServer();




    // std::cout << "last char: " << line.back() << std::endl;
    
    std::vector<std::string> knownMethods;
    knownMethods = getStringTokens("GET POST DELETE");
    
 
    Parser::lex()->set_input(line);

    this->method = Parser::lex()->next_token(true);
    this->url = Parser::lex()->next_token(true);
    this->version = Parser::lex()->next_token(true);


    if (std::find(knownMethods.begin(), knownMethods.end(), this->method) == knownMethods.end())
        throw std::runtime_error(E405);

    if (this->url.size() > 2048)
        throw std::runtime_error(E414);
    if (checkValidChars(this->url) == 0)
        throw std::runtime_error(E400);

    if (this->version.find("HTTP/1.1") == std::string::npos && this->version.find("HTTP/1.0") == std::string::npos)
        throw std::runtime_error(E505);

}

void Request::ParseHeaders(std::string &line)
{
    std::cout << CYAN << "STATE: " << (this->state == HEADERS ? "HEADERS" : "weird") << RESET << std::endl;

    if (line == "\r\n" || line == "\n")
        return;

    std::pair<std::string, std::vector<std::string> > pair;
    std::string key;
    std::string value;
    Parser::lex()->set_input(line);

    key = Parser::lex()->next_token(true);
    value = &line[key.size() + 1];

    if (key.back() != ':')
        throw std::runtime_error(E400);

    if (key == "Host:" )
    {
        this->host = value;
        this->client = &Servme::getCore()->map_clients[this->client_fd]; //TODO: change this to be in the constructor 
        this->client->selectServer();

    }


    if (key == "Content-Length:")
    {
        Parser::lex()->set_input(value);
        if (Parser::lex()->next_token(false).find_first_not_of("0123456789") != std::string::npos)
            throw std::runtime_error(E400);
        value = Parser::lex()->next_token(false);
        this->contentLength = atoi(value.c_str());
        if (this->contentLength > this->client->server->client_max_body_size)
            throw std::runtime_error(E413);
    }

    if (key == "Transfer-Encoding:")
    {
        if (value.find("chunked") != std::string::npos)
        {
            this->bodyType = BodyType::CHUNKED;
        }
        
        this->transferEncoding = value;
    }
    if (key == "Content-Type:")
    {
		this->contentType = value;
        if (value.find("multipart/form-data") != std::string::npos)
        {

            this->boundary = value.substr(value.find("boundary=") + 9);
            Parser::lex()->set_input(this->boundary);
            boundary = Parser::lex()->next_token(true);
            this->bodyType = BodyType::MULTIPART;

        }
        else {
            this->contentType = value;
        }
    }

    if (key == "connection:")
        this->connection = value;

    this->headers.insert(std::make_pair(key, value));

}


void Request::ParseBody()
{
    if (this->state == Stat::END || this->method == "GET")
        return;
    // std::cout << CYAN << "STATE: " << (this->state == BODY ? "BODY normal" : "weird") << RESET << std::endl;
    static int bodySize = 0;
    char buffer[1024];

    int bytesRead = recv(this->client_fd, buffer, std::min((this->contentLength - bodySize) , 1024), 0);

    if (bytesRead == -1)
        throw std::runtime_error("Error: read() failed. from ParseBody");
    if (bytesRead == 0)
    {
        Servme::getCore()->map_clients[this->client_fd].pollfd_.fd = -1;
        // this->client->pollfd_.fd = -1;
        std::cout << RED << "END" << RESET << std::endl;
        this->state = Stat::END;
        return;
    }
        // throw std::runtime_error("Error: read() returned 0.");
    this->bodyString += std::string(buffer, bytesRead);

    if (this->bodyType != BodyType::CHUNKED && this->bodyType != BodyType::MULTIPART)
    {
        if ((int)this->bodyString.size() >= this->contentLength)
        {
            this->bodyString = this->bodyString.substr(0, this->contentLength);
            std::cout << RED  << this->bodyString.size() << RESET << std::endl;
            std::cout << RED << "END" << RESET << std::endl;
            this->state = Stat::END;
        }
    }


    // std::cout << RED << this->bodyString << RESET << std::endl;

}








std::string nextLine(int &pos , std::string &str)
{
    std::string line;
    if (pos >= (int)str.size())
        return "EOF";
    while(str[pos] != '\r' && str[pos] != '\n' && pos < (int)str.size())
    {
        line += str[pos];
        pos++;
    }
    if (str[pos] == '\r')
        pos += 2;
    else
        pos++;
    return line;
}





void Request::ParseChunkedBody() {
    static size_t chunkSize = 0;
    static int pos = 0;
    static std::string data;

    if (this->state == Stat::END) {
        std::cout << "STAT: END" << std::endl;
        return;
    }

    if (this->state & Stat::CHUNKED_START)
    {
        std::cout << BOLDYELLOW << "STAT: CHUNKED START" << RESET << std::endl;
        this->state = Stat::CHUNKED_SIZE;
    }




    if (this->state & Stat::CHUNKED_SIZE) {

        std::cout << BOLDYELLOW << "STAT: CHUNKED SIZE"  << RESET << std::endl;
        std::string line ;
        line = nextLine(pos , this->bodyString);
        // std::getline(ss, line);
        // line.erase(line.find_first_of("\r\n"), std::string::npos);
        // std::cout << "line: " << line  << "size: " << line.size() << std::endl;
        if (line == "EOF" || this->bodyString == "\r\n" || this->bodyString == "\n")
            return;
        std::cout << "line: " << line << std::endl;
        if (line.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos)
            throw std::runtime_error("Error: invalid chunk size.");
        chunkSize = strtol(line.c_str(), NULL, 16);
        // std::cout << "bodyString: " << &this->bodyString[pos] << std::endl;
        // std::cout << "chunkSize: " << chunkSize << std::endl;
        if (chunkSize == 0)
        {
            this->bodyString = data;
            std::cout << GREEN << "BODY: " << data << std::endl;
            this->state = Stat::END;
            return;
        }
        this->state = Stat::CHUNKED_DATA;
        
    }


    if (this->state & Stat::CHUNKED_DATA) {
    
        std::cout << BOLDYELLOW << "STAT: CHUNKED DATA" << RESET << std::endl;
        // std::cout << "line: " << nextLine(pos , this->bodyString) << std::endl;

        
        std::string line;

        while(1)
        {
            line = nextLine(pos , this->bodyString);
            if (line == "EOF")
                return;
            while(chunkSize > 0 && line.size() > 0)
            {
                data += line[0];
                line.erase(0, 1);
                chunkSize--;
            }
            if (chunkSize == 0)
            {
                std::cout << BLUE <<  "chunk has been read " << data <<RESET << std::endl;
                this->state = Stat::CHUNKED_SIZE;
                break;
            }
        }
    }
}










void Request::ParseMultiPartBody()
{
    std::cout << GREEN << "BODY: " << this->bodyString << RESET << std::endl;
    static int pos = 0;
    static std::string data = "";
    static std::string fieldname = "";
    static std::string filename = "";
    static std::string ContentType = "";


    std::string line = "";


    if (this->state & Stat::MULTI_PART_START)
    {
        std::cout << "STATE: MULTI_PART_START" << std::endl;
        this->state = Stat::MULTI_PART_BOUNDARY;
    }



    while(pos < (int)this->bodyString.size())
    {
        line = nextLine(pos , this->bodyString);
        if (line == "EOF")
            return;


        if (this->state & Stat::MULTI_PART_BOUNDARY)
        {
            std::cout << "STATE: MULTI_PART_BOUNDARY" << std::endl;
            if (line.find(this->boundary) != std::string::npos)
            {
                std::cout << "switching to MULTI_PART_HEADERS" << std::endl;
                this->state = Stat::MULTI_PART_HEADERS;
            }
        }


        else if (this->state & Stat::MULTI_PART_HEADERS)
        {
            Parser::lex()->set_input(line);
            if (Parser::match("Content-Disposition:") && Parser::match("form-data;"))
            {
                    std::cout << YELLOW << "Content-Disposition: form-data;"  << RESET << std::endl;
                    fieldname = Parser::lex()->next_token(true);
                    filename = Parser::lex()->next_token(true);
                    if (fieldname.find("name=") != std::string::npos)
                    {

                            fieldname.erase(0, fieldname.find_first_of("\"") + 1);
                            fieldname.erase(fieldname.find_last_of("\""));
                            std::cout << "fieldname: " << fieldname << std::endl;
                    }
                    else if (fieldname == "EOF")
                        fieldname = "";

                    if (filename.find("filename=") != std::string::npos)
                    {
                            filename.erase(0, filename.find_first_of("\"") + 1);
                            filename.erase(filename.find_last_of("\""));
                    }   
                    else if (filename == "EOF")
                        filename = "";
            }
            if (Parser::match("Content-Type:"))
            {
                ContentType = Parser::lex()->next_token(false);
                ContentType.pop_back();
                if (ContentType == "EOF")
                    ContentType = "";
            }

            if (line == "")
            {
                std::cout  << "fieldname: " << fieldname << std::endl;
                std::cout << "filename: " << filename << std::endl;
                std::cout << "ContentType: " << ContentType <<RESET<< std::endl;
                this->multipart_env[fieldname] = Multipart_ENV(filename , ContentType);
                std::cout << BLUE << "switching to MULTI_PART_DATA" << RESET << std::endl;
                this->state = Stat::MULTI_PART_DATA;
            }
        }

        else if (this->state & Stat::MULTI_PART_DATA)
        {
            std::cout << "STATE: MULTI_PART_DATA" << std::endl;
            if (line.find(this->boundary + "--") != std::string::npos)
            {
                this->multipart_env[fieldname].data += data;
                data = "";
                std::cout << "end " << std::endl;
                this->state = Stat::END;

            }
            else if (line.find(this->boundary) != std::string::npos)
            {
                this->multipart_env[fieldname].data += data;
                std::cout << GREEN << "DATA: " << data << RESET << std::endl;
                data = "";
                this->state = Stat::MULTI_PART_HEADERS;
            }
            else
                data += line;   
        }

        if (this->state & Stat::END)
        {
            std::cout << "this is the end" << std::endl;
            // for (auto part: this->multipart_env)
            // {   std::cout << "---------------------------------" << std::endl;
            //     std::cout << "fieldname: " << part.first << std::endl;
            //     std::cout << "filename: " << part.second.file_name << std::endl;
            //     std::cout << "ContentType: " << part.second.content_type << std::endl;
            //     std::cout << "data: " << part.second.data << std::endl;
            // }
            return;
        }
        
    }


}