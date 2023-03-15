#include "../inc/client.hpp"

Client::Client()
{
    this->addr = new sockaddr_in;
    // this->response = new Response();

    this->request.core = this->core;
    this->request.client = this;
    // this->response->core = this->core;
    this->pollfd_.fd = -1;
}

Client::~Client()
{
    // close(this->fd);
    // delete this->addr;
    // delete this->request;
    // delete this->response;
}

Client::Client(SocketWrapper &sock)
{
    this->socket = &sock;
    addr = new sockaddr_in;
    fd = sock.accept(*addr);
    if (fd == -1)
    {
        throw std::runtime_error("Failed to accept connection");
    }
    fcntl(fd, F_SETFL, O_NONBLOCK);
    pollfd_.fd = fd;
    pollfd_.events = POLLIN;
}

std::string Request::checkType(std::string path)
{
    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos)
    {
        std::cout << "No extension" << std::endl;
        return "";
    }
    else
    {
        std::string extension = path.substr(dot + 1);
        std::map<std::string, std::string>::iterator iter;

        if (this->core->mimeTypes[extension] != "")
            return (this->core->mimeTypes[extension]);
        return ("");
    }
}

std::string checkForEnd(char c , int type)
{
    static int count = 0;
    if (type == 1)
    {
        if (c == '\r')
            count++;
        else if (c == '\n' && count == 2)
            count++;
        else if (c == '\r' && count == 3)
            count++;
        else if (c == '\n' && count == 4)
        {
            count = 0;
            return "\r\n\r\n";
        }
        else
            count = 0;
        return "";
    }
    else 
    {
        if (c == '\r')
            count++;
        else if (c == '\n' && count == 1)
        {
            count = 0;
            return "\r\n";
        }
        else
            count = 0;
        return "";
    }
}



void Client::handleRequest()
{
    if (this->request.state == FIRSTLINE || this->request.state == HEADERS)
    {
        static std::string line = "";
        char buffer[1];
        int ret;
        ret = recv(this->fd, buffer, 1, 0);
        if (ret == -1)
        {
            // handle error
            std::cerr << "Error: recv() failed" << std::endl;
            // for(auto i : this->core->pollFds)
            // {
            //     if (i.fd == this->fd)
            //     {
            //         i.fd = -1;
            //         break;
            //     }
            // }

            return;
        }
        else if (ret == 0)
        {
            // disconnection
            std::cout << "disconnection" << std::endl;
            for(auto i : this->core->pollFds)
            {
                if (i.fd == this->fd)
                {
                    i.fd = -1;
                    break;
                }
            }
            return;
        }

        line += buffer[0];
        this->request.buffer += buffer[0];

        if (line.find("\r\n") != std::string::npos || line.find("\n") != std::string::npos)
        {
            if (this->request.state == FIRSTLINE)
            {
                this->request.ParseFirstLine(line);
                this->request.state = HEADERS;
            }
            else if (this->request.state == HEADERS)
            {
                this->request.ParseHeaders(line);
                if (line == "\r\n" || line == "\n")
                    this->request.state = BODY;
            }
            line = "";
        }

        if (this->request.buffer.find("\r\n\r\n") != std::string::npos)
            this->request.state = BODY;
    }
    else if (this->request.state == BODY)
    {
        char buffer[1024];
        int ret;
        ret = recv(this->fd, buffer, 1024, 0);
        if (ret == -1)
        {
            // handle error
            std::cerr << "Error: recv() failed" << std::endl;
            return;
        }
        else if (ret == 0)
        {
            // disconnection
            std::cout << "disconnection" << std::endl;
            for(auto i : this->core->pollFds)
            {
                if (i.fd == this->fd)
                {
                    i.fd = -1;
                    break;
                }
            }
            return;
        }
        this->request.buffer += std::string(buffer, ret);
        this->request.ParseBody();
        // generateResponse();
        // writeResponse();
    }
}




// void Client::handleRequest()
// {
//     // std::cout << "handling request ..." << std::endl;
//     if (this->request.state == FIRSTLINE || this->request.state == HEADERS)
//     {

//         static std::string line = "";
//         char buffer[1];
//         int ret;
//         ret = recv(this->fd, buffer, 1, 0);
//         if (ret == -1)
//         {
//             // handle error
//             std::cerr << "Error: recv() failed" << std::endl;
//             // for(auto i : this->core->pollFds)
//             // {
//             //     if (i.fd == this->fd)
//             //     {
//             //         i.fd = -1;
//             //         break;
//             //     }
//             // }

//             return;
//         }
//         else if (ret == 0)
//         {
//             // disconnection
//             std::cout << "disconnection" << std::endl;
//            for(auto i : this->core->pollFds)
//             {
//                 if (i.fd == this->fd)
//                 {
//                     i.fd = -1;
//                     break;
//                 }
//             }
//             return;
//         }


//         if (checkForEnd(buffer[0] , 1) == "\r\n\r\n" && this->request.state == HEADERS)
//             this->request.state = BODY;
//         // if (line.find("\r\n") != std::string::npos || line.find("\n") != std::string::npos)
//         if (buffer[0] == '\n' || checkForEnd(buffer[0] , 0) == "\r\n")
//         {
//             if (this->request.state == FIRSTLINE)
//                 this->request.ParseFirstLine(line);
//             else if (this->request.state == HEADERS)
//                 this->request.ParseHeaders(line);
//             line = "";
//         }
//         line += buffer[0];
//         this->request.buffer += buffer[0];
//     }
//     else if (this->request.state == BODY)
//     {
//         // char buffer[1024];
//         // if (recv(this->fd, buffer, 1024, 0) == -1)
//         // {
//         //     // handle error
//         //     std::cerr << "Error: recv() failed" << std::endl;
//         // }
//         // else if (recv(this->fd, buffer, 1024, 0) == 0)
//         // {
//         //     // disconnection
//         //     std::cout << "disconnection" << std::endl;
//         // }

//         // this->request.buffer += buffer;
//         // this->request.body << buffer;
//         this->request.ParseBody();
//         // generateResponse();
//         // writeResponse();
//     }
// }