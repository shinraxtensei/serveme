#include "../inc/servme.hpp"



// #include <vector>

Client::Client()
{
    //std::cout << " Client created without socket\n";
    this->lastActivity = time(NULL);

    this->addr = new sockaddr_in;

    this->request = new Request();
    this->request->core = this->core;
    this->request->client = this;

    this->location = NULL;
    this->response = new Response();
    this->response->http = this->core->get_http();
    this->response->client = this;

    this->pollfd_.fd = -1;
}







Client::~Client()
{
 

    if (this->pollfd_.fd != -1)
    {
        close(this->pollfd_.fd);
        this->pollfd_.fd = -1;
    }
    if (this->socket != NULL)
    {
        delete this->socket;
        this->socket = NULL;
    }

    if (this->addr != NULL)
    {
        delete this->addr;
        this->addr = NULL;
    }
    if (this->request != NULL)
    {
        delete this->request;
        this->request = NULL;
    }
    if (this->response != NULL)
    {
        delete this->response;
        this->response = NULL;
    }


    if (this->cgi != NULL)
    {
        delete this->cgi;
        this->cgi = NULL;
    }
    if (this->server != NULL)
    {
        delete this->server;
        this->server = NULL;
    }
    if (this->location != NULL)
    {
        delete this->location;
        this->location = NULL;
    }

}

Client::Client(SocketWrapper *sock)
{
    
    //std::cout << " Client created with socket\n";
    // //std::cout << "memory address" << this << std::endl;
    this->lastActivity = time(NULL);


    this->cgi = new Cgi();
    this->request = new Request();
    this->response = new Response();
    this->request->client = this;
    this->response->client = this;

    this->request->core = this->core;
    this->server = NULL;
	this->location = NULL;

    // this->response->http = this->core->get_http();

    this->socket = sock;
    addr = new sockaddr_in;
    fd = sock->accept(*addr);
    if (fd == -1)
    {
        throw std::runtime_error("Failed to accept connection");
    }

    this->request->client_fd = fd;
    this->response->client_fd = fd;
    // this->request->core = Servme::getCore();
    // this->request->client = &Servme::getCore()->map_clients[fd];

    fcntl(fd, F_SETFL, O_NONBLOCK);
    pollfd_.fd = fd;
    pollfd_.events = POLLIN;
    pollfd_.revents = 0;
}





std::string Request::checkType(std::string path)
{
    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos)
    {
        //std::cout << "No extension" << std::endl;
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









void Client::handleRequest()
{   

    this->lastActivity = time(NULL);

    if (this->request->state & (START | FIRSTLINE | HEADERS))
    {

        static std::string line = "";
        char buffer[1];
        int ret = 0;
		if (this->fd != -1)
        	ret = recv(this->fd, buffer, 1, 0);
        if (ret == -1)
        {
            std::cerr << "Error: recv() failed" << std::endl;
            this->core->removeClient(*this);
    
        }
        else if (ret == 0)
        {
            //std::cout << "disconnection" << std::endl;
            this->core->removeClient(*this);
        }
        this->request->line += buffer[0];
        this->request->buffer += buffer[0];

        if ((this->request->line.find("\r") != std::string::npos || this->request->line.find("\n") != std::string::npos || this->request->line.find("\r\n") != std::string::npos ) && this->request->state & START)
        {
            this->request->line = "";
            return;
        }
        if (this->request->state & START)
            this->request->state = FIRSTLINE;
        if (this->request->line.find("\r\n") != std::string::npos || this->request->line.find("\n") != std::string::npos)
        {
            if (this->request->line == "\r\n" || this->request->line == "\n")
            {
                this->response->GENERATE_RES = true;
                this->request->state = BODY;
            }
            if (this->request->state & FIRSTLINE)
            {
                this->request->ParseFirstLine(this->request->line);
                this->request->state = HEADERS;
            }
            else if (this->request->state & HEADERS)
            {
                this->request->ParseHeaders(this->request->line);
            }
            this->request->line = "";
        }
        if (this->request->buffer.find("\r\n\r\n") != std::string::npos)
            this->request->state = BODY;
    }

    else if (this->request->state & BODY)
    {        
        //std::cout << CYAN << "STATE: " << (this->request->state == BODY ? "BODY" : "weird") << RESET << std::endl;
        

        static int flag = 0;

        

        try {
            this->request->ParseBody();
        }
        catch (const std::exception& e)
        {

            if (std::string(e.what()) == "Disconnected")
            {
                //std::cout <<  "disconnection" << std::endl;
                this->core->removeClient(*this);
            }
                //std::cout << e.what() << std::endl;
        }

        if (this->request->bodyType == CHUNKED)
        {

			
            if (flag == 0)
            {
                flag = 1;
                //std::cout << BLUE <<"this is the start of chunked body" << RESET << std::endl;
                this->request->state = CHUNKED_START;
            }
            this->request->ParseChunkedBody(); 
        }

        if (this->request->bodyType == MULTIPART)
        {
			//std::cout << "state : " << this->request->state << std::endl;
			//std::cout << "body type : multipart" << std::endl;
            if (flag == 0)
            {
                flag = 1;
                this->request->state = MULTI_PART_START;
            }
            this->request->ParseMultiPartBody(); 
        }
 
    }


}

void Client::generateResponse()
{
	//std::cout << "in generateResponse" << std::endl;
    this->lastActivity = time(NULL);
	this->response->checkCgi();
	if (this->cgiFlag == 1)
        cgi_handler();
	else
		this->response->handleNormalReq();
}

void	Client::selectServer()
{
    std::vector<Server>::iterator it;
    std::vector<Server> candidates;


    for (it = Servme::getCore()->get_http()->servers.begin();  it != Servme::getCore()->get_http()->servers.end(); it++)
    {
        if (it->ipPort.second == this->socket->get_listenPair().second)
            candidates.push_back(*it);
    }
	if (candidates.size() == 0)
		throw std::runtime_error("Error: No server found for this request.");
	else
	{
		for (it = candidates.begin(); it != candidates.end(); it++)
		{
			if (it->server_name == this->request->host)
			{
				this->server = new Server(*it);
				return ;
			}
		}
		this->server = new  Server(candidates[0]);
  }
}
