#include "../inc/core.hpp"

Multipart_ENV::Multipart_ENV()
{

}

Multipart_ENV::Multipart_ENV(std::string filename, std::string data)
{
    this->file_name = filename;
    this->data = data;
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

    std::pair<std::string, std::vector<std::string> > pair;
    std::string key;
    std::string value;
    Parser::lex()->set_input(line);

    key = Parser::lex()->next_token(true);
    value = &line[key.size() + 1];

    if (key.back() != ':')
        std::cout << "Error: Invalid header line." << std::endl;

    if (key == "Host:" )
    {
        this->host = value;
        this->client = &Servme::getCore()->map_clients[this->client_fd]; //TODO: change this to be in the constructor 
        this->client->selectServer();

    }


    if (key == "Content-Length:")
    {
        // this->bodyType = BodyType::NONE;
        Parser::lex()->set_input(value);
        if (Parser::lex()->next_token(false).find_first_not_of("0123456789") != std::string::npos)
            std::cout << "Error: Invalid content-length." << std::endl;
        value = Parser::lex()->next_token(false);
        this->contentLength = atoi(value.c_str());
        if (this->contentLength > this->client->server->client_max_body_size)
            throw std::runtime_error("Error: Content-Length is too big.");
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
        if (value.find("multipart/form-data") != std::string::npos)
        {

            this->boundary = value.substr(value.find("boundary=") + 9);
            Parser::lex()->set_input(this->boundary);
            boundary = Parser::lex()->next_token(true);
            this->bodyType = BodyType::MULTIPART;       

        }
    }

    if (key == "connection:")
        this->connection = value;

    this->headers.insert(std::make_pair(key, value));

}

void Request::ParseBody()
{
    if (this->state == Stat::END)
        return;
    // std::cout << CYAN << "STATE: " << (this->state == BODY ? "BODY normal" : "weird") << RESET << std::endl;
    if (this->state == Stat::END)
        return;
    static int bodySize = 0;
    char buffer[1024];

    // std::cout << YELLOW  << "content-length: " << this->contentLength << RESET << std::endl;
    int bytesRead = read(this->client_fd, buffer, std::min((this->contentLength - bodySize), 1024));
    if (bytesRead == -1)
        throw std::runtime_error("Error: read() failed.");
    if (bytesRead == 0)
    {
        this->state = Stat::END;
        return;
    }
        // throw std::runtime_error("Error: read() returned 0.");
    this->bodyString += std::string(buffer, bytesRead);

    if (this->bodyType != BodyType::CHUNKED)
    {
        if ((int)this->bodyString.size() >= this->contentLength)
        {
            this->bodyString = this->bodyString.substr(0, this->contentLength);
            std::cout << RED  << this->bodyString.size() << RESET << std::endl;
            std::cout << RED << "END" << RESET << std::endl;
            this->state = Stat::END;
        }
    }

    // std::cout << this->bodyString << std::endl;
}





// void Request::ParseChunkedBody() {
//     static size_t chunkSize = 0;
//     static std::string data;
//     // static int bytesRead = 0;

//     if (this->state == Stat::END) {
//         std::cout << "STAT: END" << std::endl;
//         return;
//     }

//     if (this->state & Stat::CHUNKED_START)
//     {
//         std::cout << "STAT: CHUNKED START" << std::endl;
//         this->state = Stat::CHUNKED_SIZE;
//     }




//     if (this->state & Stat::CHUNKED_SIZE) {

//         std::cout << "STAT: CHUNKED SIZE" << std::endl;


//         Parser::lex()->set_input(this->bodyString);
//         std::string line = Parser::lex()->next_token(true);
//         if (line == "EOF" || this->bodyString == "\r\n" || this->bodyString == "\n")
//             return;
//         std::cout << "line: " << line << std::endl;
//         if (line.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos)
//             throw std::runtime_error("Error: invalid chunk size.");
//         chunkSize = strtol(line.c_str(), NULL, 16);
//         if (this->bodyString[0] == '\r')
//             this->bodyString = this->bodyString.substr(line.size() + 2);
//         else
//             this->bodyString = this->bodyString.substr(line.size() + 1);
//         std::cout << "chunkSize: " << chunkSize << std::endl;
//         if (chunkSize == 0)
//         {
//             // this->bodyString = data;
//             std::cout << GREEN << "BODY: " << data << std::endl;
//             std::cout << "END" << RESET << std::endl;
//             this->state = Stat::END;
//             return;
//         }
//         this->state = Stat::CHUNKED_DATA;

//     }

//     if (this->state & Stat::CHUNKED_DATA) {
//         // Parse the chunk data
//         static int pos = 0;
        
//         std::cout << "STAT: CHUNKED DATA" << std::endl;
        
//         std::cout << RED << "chunkSize: " << chunkSize <<RESET << std::endl;
//         std::cout << RED << "bodyString :  " << this->bodyString << RESET << std::endl;
//         std::cout << RED << "bodyString.size(): " << this->bodyString.size() << RESET << std::endl;
//         std::stringstream ss;
//         this->bodyString.substr(pos);
//         ss << this->bodyString;
//         std::string line;






        // if (chunkSize > this->bodyString.size())
        // {
        //     data += this->bodyString;
        //     this->bodyString = "";
        //     chunkSize -= this->bodyString.size();
        //     return;
        // }
        // else
        // {
            // while(Parser::lex()->input_stream.peek() != EOF  && chunkSize > 0)
            // {
            //     data += Parser::lex()->input_stream.get();
            //     if (data.back() == '\r' || data.back() == '\n')
            //         chunkSize++;
            //     chunkSize--;
            //     if (chunkSize == 0)
            //     {
            //         this->bodyString = "";
            //         this->state = Stat::CHUNKED_SIZE;
            //         break;
            //     }
            // }
        // }
    // }

// }
// static std::string nextLine(std::string &str)
// {
//     std::string line;
//     std::stringstream ss;
//     ss << str;
//     std::getline(ss, line);

//     str = ss.str();
//     return line;
// }


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
    static std::stringstream ss;
    ss << this->bodyString;

    // static int bytesRead = 0;

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
            std::cout << GREEN << "BODY: " << this->bodyString << std::endl;
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
                std::cout << BLUE <<  "chunk has been read !!" << data <<RESET << std::endl;
                this->state = Stat::CHUNKED_SIZE;
                break;
            }
        }
    }
}



































void Request::ParseMultiPartBody()
{
    if (this->state & Stat::END)
        return;
    std::cout << CYAN << "STATE: " << (this->state & BODY ? "BODY multiPart" : "weird") << RESET << std::endl;

// // ? : i should  read with a 1024 buffer , while keeping track of the content-length
// // ? : after reading i should give the buffer to a stringstream and parse it
// // ? : the first state will be the boundary since its the first thing in the body after the headers
    char buffer[1024];
    // static int bytesRead = 0;
    int bytesRead = read(this->client_fd, buffer, 1024);
    if (bytesRead == -1)
    {
        throw std::runtime_error("Error: read() failed.");
    }
    if (bytesRead == 0)
    {
        std::cout << "READ 0" << std::endl;
        // throw std::runtime_error("Error: read() returned 0.");
    }
    this->bodyString += std::string(buffer, bytesRead);
    Parser::lex()->set_input(this->bodyString);
    // std::string str(buffer, bytesRead);
    // std::cout << "buffer: " << str.substr(bytesRead) << std::endl;
    // Parser::lex()->set_input(std::string(buffer + bytesRead , bytesRead));

    // std::cout << Parser::lex()->next_token(false) << std::endl;



    if (this->state & Stat::MULTI_PART_START)
    {
        std::cout << "STATE: MULTI_PART_START" << std::endl;
        this->state = Stat::MULTI_PART_BOUNDARY;
    }



    if (this->state & Stat::MULTI_PART_BOUNDARY)
    {
        std::cout << "STATE: MULTI_PART_BOUNDARY" << std::endl;
        std::cout << "bonudary from headers is : " << this->boundary << std::endl;
        // Read data from socket
        if (this->state & Stat::END)
            return;
        if (Parser::match(this->boundary))
            this->state = Stat::MULTI_PART_HEADERS;


    }

    else if (this->state & Stat::MULTI_PART_HEADERS)
    {

        static std::string fieldname = "";
        static std::string filename = "";
        static std::string ContentType = "";
        static std::string content = "";

        std::cout << "STATE: MULTI_PART_HEADERS" << std::endl;
        if (Parser::match("Content-Disposition:") && Parser::match("form-data;"))
        {
            std::cout << "Content-Disposition: form-data;" << std::endl;
            if (Parser::lex()->next_token(false).find("name=") != std::string::npos)
            {
                std::cout << "name=" << std::endl ;
                fieldname = Parser::lex()->next_token(false);
                // std::string name = Parser::match_string();
                // std::cout << "name: " << name << std::endl;
                if (Parser::match(";"))
                {
                    std::cout << ";" << std::endl;
                    if (Parser::match("filename="))
                    {
                        std::cout << "filename=" << std::endl;
                        // std::string filename = Parser::match_string();
                        // std::cout << "filename: " << filename << std::endl;
                        if (Parser::match("\r\n\r\n"))
                        {
                            std::cout << "\r\n\r\n" << std::endl;
                            this->state = Stat::MULTI_PART_DATA;
                        }
                    }
                }
                else if (Parser::match("\r\n\r\n"))
                {
                    std::cout << "\r\n\r\n" << std::endl;
                    this->state = Stat::MULTI_PART_DATA;
                }
            }
        }
     

        // this->state = Stat::MULTI_PART_DATA;

    }
    


    
    else if (this->state & Stat::MULTI_PART_DATA)
    {
        std::cout << "STATE: MULTI_PART_DATA" << std::endl;

        


        this->state = Stat::MULTI_PART_BOUNDARY;
    }


}












// void Request::ParseMultiPartBody()
// {
//     if (this->state & Stat::END)
//         return;

//     // Initialize a buffer to read the body data
//     char buffer[1024];
//     int bytes_read;

//     std::cout << CYAN << "STATE: " << (this->state & BODY ? "BODY multiPart" : "weird") << RESET << std::endl;

//     if (this->state & Stat::MULTI_PART_START)
//     {
//         std::cout << "STATE: MULTI_PART_START" << std::endl;

//         // Initialize a string stream to parse the body data
//         std::stringstream ss;
//         this->.

//         // Set the current boundary for the body data

//         this->state = Stat::MULTI_PART_BOUNDARY;
//     }

//     if (this->state & Stat::MULTI_PART_BOUNDARY)
//     {
//         std::cout << "STATE: MULTI_PART_BOUNDARY" << std::endl;

//         // Read the next chunk of data from the request body

//         if (bytes_read <= 0) {
//             // End of request body
//             // this->state |= Stat::END;
//             return;
//         }

//         // Append the data to the current multipart data string
//         // this->multi_part_data.append(buffer, bytes_read);

//         // Check if the current boundary is found in the data
//         // size_t boundary_pos = this->multi_part_data.find(this->multi_part_boundary);
//         // if (boundary_pos == std::string::npos) {
//             // The boundary is not found yet, continue reading
//             // return;
//         }

//         // Trim any remaining data after the boundary
//         // this->multi_part_data.erase(boundary_pos);

//         this->state = Stat::MULTI_PART_HEADERS;
//     }

//     if (this->state & Stat::MULTI_PART_HEADERS)
//     {
//         std::cout << "STATE: MULTI_PART_HEADERS" << std::endl;

//         // Parse the headers for the current part of the multipart data
//         // std::istringstream ss(this->multi_part_data);
//         // std::string line;
//         // while (std::getline(ss, line) && !line.empty())
//         // {
//             // std::cout << "Header line: " << line << std::endl;
//             TODO: Process the header line
//         // }

//         // Remove the headers from the multipart data
//         // size_t pos = this->multi_part_data.find("\r\n\r\n");
//         // if (pos != std::string::npos) {
//             // this->multi_part_data.erase(0, pos + 4);
//         // }

//         this->state = Stat::MULTI_PART_DATA;
//     }

//     if (this->state & Stat::MULTI_PART_DATA)
//     {
//         std::cout << "STATE: MULTI_PART_DATA" << std::endl;

//         // Process the data for the current part of the multipart data
//         // TODO: Process the data using the Content-Disposition and Content-Type headers

//         // Set the state back to reading the next boundary
//         this->state = Stat::MULTI_PART_BOUNDARY;
//     }
// }


