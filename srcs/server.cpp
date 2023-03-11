#include "../inc/core.hpp"




// ! : not used for now but can have important impolementations , so not deleting it for now 

// SocketWrapper *Server::Socket()
// {
//     if (this->sock == nullptr)
//         this->sock = new SocketWrapper(AF_INET , SOCK_STREAM , 0);
//     std::cout << this->sock->get_sockfd() << std::endl;
//     return this->sock;
// }


// Server::Server()
// {
    // Socket();
// }

// Server::~Server()
// {
    // delete this->sock;
// }

// void Server::connect()
// {   
    // try
    // {    
        
    //     if (this->server_directives.find("listen") != this->server_directives.end())
    //     {
    //         this->listen = std::stoi(this->server_directives["listen"][0]);
    //         std::cout << "listen: " << this->listen << std::endl;
    //         this->sock->bind(this->listen);
    //     }
    //     else
    //     {
    //         std::cout << "Error: listen directive not found" << std::endl;
    //         exit(1);
    //     }
    //     this->sock->listen(100);
    // }
    // catch(const std::exception& e)
    // {
    //     std::cerr << e.what() << '\n';
    // }
// }

// void Server::HandleRequest( int fd)
// {
// 	char buff[1024];
// 	while(read(fd, buff, 1024) > 0);
// 	std::cout << buff << std::endl;



	// (void) fd;
	
	// std::vector<std::string> lines;
	// std::string key;
	// std::vector<std::string> values;
	// std::string	line; 
	// std::string	buffer;
	// std::ifstream	file("/Users/yabtaour/Desktop/webserv-42/request");
	// while (std::getline(file, line))
	// {
	// 	buffer += line;
	// 	buffer += "\n";
	// 	if (line.size() > 0)
	// 		lines.push_back(line + '\n');
	// }
	// std::cout << buffer << std::endl;



// 	for(size_t i = 0 ; i < lines.size() ; i++)
// 	{
// 		Parser::lex()->set_input(lines[i]);
// 		if (i == 0)
// 		{
// 			key = "Method";
// 			values.push_back(Parser::lex()->next_token(true));
// 			std::pair<std::string , std::vector<std::string> > pair(key , values);
// 			this->request.insert(pair);
// 			values.clear();
// 			key = "Path";
// 			values.push_back(Parser::lex()->next_token(true));
// 			pair.first = key ; pair.second = values;
// 			this->request.insert(pair);
// 			values.clear();
// 			key = "Version";
// 			values.push_back(Parser::lex()->next_token(true));
// 			pair.first = key ; pair.second = values;
// 			this->request.insert(pair);
// 			values.clear();
// 		}
// 		else
// 		{
// 			key = Parser::lex()->next_token(true);
// 			while(Parser::lex()->next_token(false) != "EOF" && Parser::lex()->next_token(false).back() != '\n' )
// 				values.push_back(Parser::lex()->next_token(true));
// 			std::pair<std::string , std::vector<std::string> > pair(key , values);
// 			this->request.insert(pair);
// 			values.clear();
// 		}
// 	}
// 	for (auto it = this->request.begin() ; it != this->request.end() ; it++)
// 	{
// 		std::cout << "key : " << it->first << std::endl;
// 		for(auto i : it->second)
// 		{
// 			std::cout << "values :" ;
// 			std::cout << i + " " ;
// 		}
// 		std::cout << std::endl;
// 	}
// }

// void Server::HandleResponse() {}

// void	Core::parseMimeTypes(void)
// {
// 	std::string buffer;
// 	// std::ifstream file("/Users/yabtaour/Desktop/webserv-42/mime.types");
// 	std::ifstream file("/Users/ahouari/triz_work/serveme/mime.types");
// 	while (std::getline(file, buffer))
// 	{
// 		if (buffer.size() > 0 && buffer[0] != '#')
// 		{
// 			std::pair<std::string, std::string> pair;
// 			std::istringstream iss(buffer);
// 			iss >> pair.first;
// 			iss >> pair.second;
// 			this->mimeTypes.insert(pair);
// 		}
// 	}
// 	// for (const auto& pair : this->mimeTypes)
//     // 	std::cout << pair.first << " => " << pair.second << std::endl;
// }

// std::string	Core::checkType(std::string	path)
// {
// 	size_t dot = path.find_last_of('.');
//     if (dot == std::string::npos) {
// 		std::cout << "No extension" << std::endl;
// 		return "";
// 	} else {
//     	std::string extension = path.substr(dot + 1);
// 		std::map<std::string, std::string>::iterator iter;
// 		// for (iter = this->mimeTypes.begin(); iter != this->mimeTypes.end(); ++iter) {
//         // 	if (iter->first == extension)
//         //     	return (iter->second);
//     	// }
// 		// if ((iter = this->mimeTypes.find(extension)) != this->mimeTypes.end())
// 		if (this->mimeTypes[extension] != "")
// 			return (this->mimeTypes[extension]);
// 		return ("");
//     }
// }