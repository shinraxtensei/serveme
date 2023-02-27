#include "../inc/core.hpp"


// SocketWrapper *Server::Socket()
// {
//     if (this->sock == nullptr)
//         this->sock = new SocketWrapper(AF_INET , SOCK_STREAM , 0);
//     std::cout << this->sock->get_sockfd() << std::endl;
//     return this->sock;
// }


Server::Server()
{
    // Socket();
}

Server::~Server()
{
    // delete this->sock;
}

void Server::connect()
{   
    try
    {    
        
        if (this->server_directives.find("listen") != this->server_directives.end())
        {
            this->listen = std::stoi(this->server_directives["listen"][0]);
            std::cout << "listen: " << this->listen << std::endl;
            this->sock->bind(this->listen);
        }
        else
        {
            std::cout << "Error: listen directive not found" << std::endl;
            exit(1);
        }
        this->sock->listen(10);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void Server::HandleRequest( int fd)
{
	(void) fd;
	// std::map<std::string, std::vector<std::string> >	;
	// std::string	key;
	// int		i = 0;

	std::vector<std::string> lines;

	std::string first;
	
	
	std::string	line; 
	std::string	buffer;
	std::ifstream	file("/Users/yabtaour/Desktop/webserv-42/request");
	while (std::getline(file, line))
	{
		buffer += line;
		buffer += "\n";
		if (line.size() > 0)
			lines.push_back(line );
	}
	for(size_t i = 0 ; i < lines.size() ; i++)
	{
		std::vector<std::string> values;
		Parser::lex()->set_input(lines[i]);

		first = Parser::lex()->next_token(true);
		while(Parser::lex()->next_token(false) != "EOF")
			values.push_back(Parser::lex()->next_token(true));
		std::pair<std::string , std::vector<std::string> > pair(first , values);
		this->request.insert(pair);
		values.clear();
	}

	for (auto it : this->request)
	{
		std::cout << "key : " << it.first << std::endl;
		for(auto i : it.second)
		{
			std::cout << "values :" ;
			std::cout << i + " " ;
		}
		std::cout << std::endl;
	}
	// Parser::lex()->set_input(buffer);
	// while(Parser::lex()->next_token(false) != "EOF")
	// {
	// 	if (i == 0)
	// 		this->request
	// 	else
	// 	while (Parser::lex()->next_token(false).back() != '\n')
	// 	{

	// 	}
	// 	std::cout << Parser::lex()->next_token(true) << std::endl;
	// 	while()

	// }

    // char buffer[1024];
    // read(fd, buffer, 1024);
    // std::cout << "Request received" << std::endl;
    // std::cout << "Request: " << buffer << std::endl;

	// std::string request = Parser::lex()->next_token(true);
	// std::cout << request << std::endl;
	// request = Parser::lex()->next_token(true);
	// std::cout << request << std::endl;
}


void Server::HandleResponse() {}
