#include "../inc/core.hpp"



void Request::ParseFirstLine()
{

}

void Request::ParseHeaders()
{

}


void Request::ParseBody()
{

}


void Request::ParseRequest()
{

    int ret = 0;

    if (this->status == FIRSTLINE || this->status == HEADERS)
    {
            char buffer[1];
        while(buffer[0] != '\n')
        {
            if (ret = recv(this->client.fd, buffer, 1, 0) == 0 ; ret == -1)
            {
                // handle error
                std::cerr << "Error: recv() failed" << std::endl;

            }
            else if (ret == 0) 
            {
                // disconnection
            }
            

            this->buffer += buffer[0];
        }
        if (this->status == FIRSTLINE)
            this->ParseFirstLine();
        else if (this->status == HEADERS)
            this->ParseHeaders();
    }
    else if (this->status == BODY)
    {
        this->ParseBody();
        // generateResponse();
        // writeResponse();

    }
        
}