#include "../inc/client.hpp"


Client::Client()
{
    this->addr = new sockaddr_in;
    this->request = new Request();
    this->response = new Response();
    this->request->core = this->core;
    // this->response->core = this->core;
    this->pollfd_.fd = -1;
}

Client::~Client()
{
    close(this->fd);
    delete this->addr;
    delete this->request;
    delete this->response;
}

Client::Client(SocketWrapper &sock)
{
        this->socket = &sock;
        addr = new sockaddr_in;
        fd = sock.accept(*addr);
        if (fd == -1) {
            throw std::runtime_error("Failed to accept connection");
        }
        fcntl(fd, F_SETFL, O_NONBLOCK);
        pollfd_.fd = fd;
        pollfd_.events = POLLIN; 
}



std::string	Request::checkType(std::string	path)
{
	size_t dot = path.find_last_of('.');
    if (dot == std::string::npos) {
		std::cout << "No extension" << std::endl;
		return "";
	} else {
    	std::string extension = path.substr(dot + 1);
		std::map<std::string, std::string>::iterator iter;
        
		// for (iter = this->mimeTypes.begin(); iter != this->mimeTypes.end(); ++iter) {
        // 	if (iter->first == extension)
        //     	return (iter->second);
    	// }
		// if ((iter = this->mimeTypes.find(extension)) != this->mimeTypes.end())


		// if (this->mimeTypes[extension] != "")
		// 	return (this->mimeTypes[extension]);

        if (this->core->mimeTypes[extension] != "")
            return (this->core->mimeTypes[extension]);
		return ("");
    }
}


void Client::handleRequest()
{
    std::cout << "handling request !!!" << std::endl;


}




// void client::handleRequest()
// {

//     int ret = 0;

//     if (this->status == FIRSTLINE || this->status == HEADERS)
//     {
//             char buffer[1];
//         while(buffer[0] != '\n')
//         {
//             if (ret = recv(this->client.fd, buffer, 1, 0) == 0 ; ret == -1)
//             {
//                 // handle error
//                 std::cerr << "Error: recv() failed" << std::endl;

//             }
//             else if (ret == 0) 
//             {
//                 // disconnection
//             }
            

//             this->buffer += buffer[0];
//         }
//         if (this->state == FIRSTLINE)
//             this->ParseFirstLine();
//         else if (this->state == HEADERS)
//             this->ParseHeaders();
//     }
//     else if (this->state == BODY)
//     {
//         this->ParseBody();
//         // generateResponse();
//         // writeResponse();

//     }
        
// }

// void Request::ParseFirstLine()
// {
    
// }

