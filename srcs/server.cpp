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
        this->sock->listen(100);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void Server::HandleRequest( int fd)
{
	(void) fd;
	int	emptyLine = 0;
	std::vector<std::string> lines;
	std::string key;
	std::vector<std::string> values;
	std::string	line; 
	std::ifstream	file("/Users/yabtaour/Desktop/webserv-42/request");
	while (std::getline(file, line))
	{
		if (line.size() > 0)
			lines.push_back(line + '\n');
		else
		{
			emptyLine++;
			std::getline(file, line);
			if (line.size() > 0)
			{
				this->request_body += line + '\n';
				while (std::getline(file, line))
					this->request_body += line + '\n';
				emptyLine++;
				std::getline(file, line);
				if (line.size() == 0)
					emptyLine++;		
			}
			else
				emptyLine++;
		}
	}
	if (emptyLine > 1)
	{
		std::cout << "Error: empty line in request" << std::endl;
		exit(1);
	}
	for(size_t i = 0 ; i < lines.size() ; i++)
	{
		Parser::lex()->set_input(lines[i]);
		if (i == 0)
		{
			key = "Method";
			values.push_back(Parser::lex()->next_token(true));
			std::pair<std::string , std::vector<std::string> > pair(key , values);
			this->request.insert(pair);
			values.clear();
			key = "Path";
			values.push_back(Parser::lex()->next_token(true));
			pair.first = key ; pair.second = values;
			this->request.insert(pair);
			values.clear();
			key = "Version";
			values.push_back(Parser::lex()->next_token(true));
			pair.first = key ; pair.second = values;
			this->request.insert(pair);
			values.clear();
		}
		else
		{
			key = Parser::lex()->next_token(true);
			while(Parser::lex()->next_token(false) != "EOF" && Parser::lex()->next_token(false).back() != '\n')
				values.push_back(Parser::lex()->next_token(true));
			std::pair<std::string , std::vector<std::string> > pair(key , values);
			this->request.insert(pair);
			values.clear();
		}
	}
	for (auto it = this->request.begin() ; it != this->request.end() ; it++)
	{
		std::cout << "key : " << it->first << std::endl;
		for(auto i : it->second)
		{
			std::cout << "values :" ;
			std::cout << i + " " ;
		}
		std::cout << std::endl;
	}
	std::cout << "request body : " << this->request_body << std::endl;
}

void Server::HandleResponse() {}
