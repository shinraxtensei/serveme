#include "../inc/client.hpp"
#include "../inc/macros.hpp"


Response::~Response()
{
	
}

int	Response::checkAccess(std::string path)
{
    std::ifstream file(path.substr(1));
    if (!file.good())
	{
		std::cout << "hadchi mal9inach" << std::endl;
        return 0;
	}
	std::stringstream content;
	content << file.rdbuf();
	this->body = content.str();
	return 1;
}

int	Response::checkError(int error)
{
	std::vector<std::string>	candidates;
	std::map<int, std::vector<std::string> >	member;
	std::string	path;
	std::string	root;

	this->client = &Servme::getCore()->map_clients[this->client_fd];
	std::cout << "in checkError" << std::endl;

	if (this->client->location)
	{
		root = this->client->location->root;
		if (!this->client->location->error_page.empty())
			member = this->client->location->error_page;
		else
		{
			std::cout << "location kaayna wlkn error pages dyalha khawyin" << std::endl;
			if (!this->client->server->error_page.empty())
				member = this->client->server->error_page;
		}
	}
	else
	{
		if (!this->client->server->error_page.empty())
			member = this->client->server->error_page;
		root = this->client->server->root;
	}
	if (member.empty())
	{
		std::cout << "makayna fhta w7da fihom" << std::endl;
		return 0;
	}
	member.find(error);
	if (member.find(error) != member.end())
	{
		std::cout << "l9inaahaa" << std::endl;
		candidates = member[error];
		for (std::vector<std::string>::iterator it = candidates.begin(); it != candidates.end(); it++)
		{
			std::cout << "ha7na ghanchoufo" << std::endl;
			path = root + "/" + *it;
			std::cout << "khasna nl9aaw" << path << std::endl;
			if (checkAccess(path))
			{
				std::cout << "kaaayna" << std::endl;
				return 1;
			}
		}
	}
	return 0;
}

void	Response::storeMimeTypes()
{
    std::vector<std::pair<std::string, std::string>> keyValuePairs = {
        {"html", "text/html"},
        {"htm", "text/html"},
        {"shtml", "shtml"},
        {"css", "text/css"},
        {"xml", "text/xml"},
        {"rss", "text/xml"},
        {"gif", "image/gif"},
        {"jpeg", "image/jpeg"},
        {"jpg", "image/jpeg"},
        {"js", "application/x-javascript"},
        {"txt", "text/plain"},
        {"htc", "text/x-component"},
        {"mml", "text/mathml"},
        {"png", "image/png"},
        {"ico", "image/x-icon"},
        {"jng", "image/x-jng"},
        {"wbmp", "image/vnd.wap.wbmp"},
        {"jar", "application/java-archive"},
        {"war", "application/java-archive"},
        {"ear", "application/java-archive"},
        {"hqx", "application/mac-binhex40"},
        {"pdf", "application/pdf"},
        {"cco", "application/x-cocoa"},
        {"jardiff", "application/x-java-archive-diff"},
        {"jnlp", "application/x-java-jnlp-file"},
        {"run", "application/x-makeself"},
        {"pl", "application/x-perl"},
        {"pm", "application/x-perl"},
        {"prc", "application/x-pilot"},
        {"pdb", "application/x-pilot"},
        {"rar", "application/x-rar-compressed"},
        {"rpm", "application/x-redhat-package-manager"},
        {"sea", "application/x-sea"},
        {"swf", "application/x-shockwave-flash"},
        {"sit", "application/x-stuffit"},
        {"tcl", "application/x-tcl"},
        {"tk", "application/x-tcl"},
        {"der", "application/x-x509-ca-cert"},
        {"pem", "application/x-x509-ca-cert"},
        {"crt", "application/x-x509-ca-cert"},
        {"xpi", "application/x-xpinstall"},
        {"zip", "application/zip"},
        {"deb", "application/octet-stream"},
        {"bin", "application/octet-stream"},
        {"exe", "application/octet-stream"},
        {"dll", "application/octet-stream"},
        {"dmg", "application/octet-stream"},
        {"eot", "application/octet-stream"},
        {"iso", "application/octet-stream"},
        {"img", "application/octet-stream"},
        {"msi", "application/octet-stream"},
        {"msp", "application/octet-stream"},
        {"msm", "application/octet-stream"},
        {"mp3", "audio/mpeg"},
		{"mp4", "video/mp4"},
        {"ra", "audio/x-realaudio"},
        {"mpeg", "video/mpeg"},
        {"mpg", "video/mpeg"},
        {"mov", "video/quicktime"},
        {"flv", "video/x-flv"},
        {"avi", "video/x-msvideo"},
        {"wmv", "video/x-ms-wmv"},
        {"asx", "video/x-ms-asf"},
        {"asf", "video/x-ms-asf"},
        {"mng", "video/x-mng"}
    };
    
	for (std::vector<std::pair<std::string, std::string>>:: iterator it = keyValuePairs.begin(); it < keyValuePairs.end(); it++)
		this->contentTypes[it->first] = it->second;
}

Response::Response()
{
	this->GENERATE_RES = false;
}

void	Response::checkAllowedMethods()
{
	std::vector<std::string>			methods;
	std::vector<std::string>::iterator	iter;

	this->client = &Servme::getCore()->map_clients[this->client_fd];
	methods = this->client->location->allowed_methods;
	if (methods.size() == 0 && this->client->request->method == "GET")
		return ;
	for (iter = methods.begin(); iter < methods.end(); iter++)
	{
		if (*iter == this->client->request->method)
			break ;
	}
	if (iter == methods.end())
	{
		if (checkError(405))
			this->responseStr = generateError(E405, DEFAULT);
		else
			this->responseStr = generateError(E405, MINE);
		send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		this->responseSent = 1;
		this->client->request->state = DONE;
		return ;
	}
}

int		LocationFound(std::string locationPaths, std::string	path)
{
	if (locationPaths == path)
			return (1) ;
	while (!path.empty() && path != "/")
	{
		if (locationPaths == path)
			return (1) ;
		path = path.substr(0, path.find_last_of("/"));
		if (path == "")
			path = "/";
		if (locationPaths == path)
			return (1) ;
	}
	return (0);
}

std::vector<Location>	getNested(std::vector<Location>	candidates, Location location)
{
	std::vector<Location>::iterator	iter;

	iter = location.locations.begin();
	for (iter = location.locations.begin(); iter < location.locations.end(); iter++)
	{
		if (location.locations.empty())
			candidates.push_back(*iter);
		else
		{
			candidates.push_back(*iter);
			getNested(candidates, *iter);
		}
	}
	return (candidates);	
}

std::vector<Location>	Response::getLocations(std::vector<Location> locations)
{
	std::vector<Location>	candidates;
	std::vector<Location>::iterator	iter;

	this->client = &Servme::getCore()->map_clients[this->client_fd];
	iter = locations.begin();
	
	for (iter = this->client->server->locations.begin(); iter < this->client->server->locations.end(); iter++)
	{
		candidates.push_back(*iter);
		candidates = getNested(candidates, *iter);
	}
	return (candidates);
}

int		countFields(std::string	path)
{
	int count = 0;
    for (std::string::const_iterator it = path.begin(); it != path.end(); ++it) {
        if (*it == '/') {
            ++count;
        }
    }
    return count;
}

bool	compareFields(const Location& loc1, const Location& loc2)
{
	return (countFields(loc1.path) > countFields(loc2.path));
}

void	Response::matchLocation(std::vector<Location> locations)
{
	std::vector<Location>	candidates;
	std::vector<Location>::iterator	iter;

	this->client = &Servme::getCore()->map_clients[this->client_fd];
	candidates = this->getLocations(locations);
	if (candidates.empty())
	{
		if (checkError(404))
			this->responseStr = generateError(E404, DEFAULT);
		else
			this->responseStr = generateError(E404, MINE);
		send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		this->responseSent = 1;
		this->client->request->state = DONE;
		return ;
	}
	std::sort(candidates.begin(), candidates.end(), compareFields);
	for (iter = candidates.begin(); iter < candidates.end(); iter++)
	{
		if (LocationFound(iter->path, this->client->request->url))
		{
			std::cout << "location fouuund" << std::endl;
			this->client->location = new Location(*iter);
			return ;
		}
	}
	if (checkError(404))
		this->responseStr = generateError(E404, DEFAULT);
	else
		this->responseStr = generateError(E404, MINE);
	send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
	this->responseSent = 1;
	this->client->request->state = DONE;
	return ;
}

void	Response::checkCgi()
{
	std::string	cgiPath = "/cgi-bin/";
	this->client = &Servme::getCore()->map_clients[this->client_fd];
	std::string	imaginaryPath = this->client->request->url;
	if (imaginaryPath.find(cgiPath) != std::string::npos)
	{
		this->client->cgiFlag = 1;
		return ;
	}
	else

		this->client->cgiFlag = 0;
}

void	Response::handleGet(int type, std::string newPath)
{
	if (type == DIRE)
	{
		if (this->responseSent == 0)
		{
    		DIR	*dir = opendir(this->client->path.c_str());
			if (dir == NULL)
			{
				if (checkError(500))
					this->responseStr = generateError(E500, DEFAULT);
				else
					this->responseStr = generateError(E500, MINE);
				send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
				this->responseSent = 1;
				this->client->request->state = DONE;
				return ;
			}		
			this->listDirectory(newPath, dir);
		}
	}
	if (type == FILE)
	{
		if (this->started == 0)
		{
			struct    stat    infos;
    		if ((stat(newPath.c_str(), &infos) != 0))
			{
				if (checkError(404))
					this->responseStr = generateError(E404, DEFAULT);
				else
					this->responseStr = generateError(E404, MINE);
				send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
				this->responseSent = 1;
				this->client->request->state = DONE;
				return ;
			}
			if (S_ISREG(infos.st_mode))
				this->contentLength = infos.st_size;
		}
		std::cout << "He wants a file" << std::endl;
        if (access(newPath.c_str(), R_OK) == 0)
			this->sendFile(newPath);
		else
		{
			if (checkError(403))
				this->responseStr = generateError(E403, DEFAULT);
			else
				this->responseStr = generateError(E403, MINE);
			send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
			this->responseSent = 1;
			this->client->request->state = DONE;
			return ;
		}
	}
}

std::string	Response::getIndex(std::string newPath)
{
	std::vector<std::string>::iterator	it;

	std::cout << newPath << std::endl;
	for (it = this->client->location->index.begin(); it < this->client->location->index.end(); it++)
	{
		if (access((newPath + "/" + (*it)).c_str(), R_OK) == 0)
			return (*it);
	}
	return ("");
}

void    Response::checkPath()
{
    struct    stat    infos;

	std::cout << "IN CHECK PATH" << std::endl;
	this->client = &Servme::getCore()->map_clients[this->client_fd];
	std::string	newPath = this->client->path.substr(this->client->path.find_first_of('/') + 1, this->client->path.length() - this->client->path.find_first_of('/') + 1);
	if ((stat(newPath.c_str(), &infos) != 0))
	{
		if (checkError(404))
			this->responseStr = generateError(E404, DEFAULT);
		else
			this->responseStr = generateError(E404, MINE);
		send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		this->responseSent = 1;
		this->client->request->state = DONE;
		return ;
	}
    if (S_ISDIR(infos.st_mode))
	{
		std::cout << "IT'S A DIRECTORY" << std::endl;
		if (this->client->request->method == "DELETE")
		{
			if (checkError(405))
				this->responseStr = generateError(E405, DEFAULT);
			else
				this->responseStr = generateError(E405, MINE);
		}
		else if (this->client->request->method == "POST")
			this->handlePost();
		else if (this->client->request->method == "GET")
		{
			if ((this->client->location->index[0] != ""))
			{
				std::cout << "there are indexes" << std::endl;
				std::string	file = this->getIndex(newPath);
				if (file == "")
				{
					if (checkError(404))
						this->responseStr = generateError(E404, DEFAULT);
					else
						this->responseStr = generateError(E404, MINE);
					send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
					this->responseSent = 1;
					this->client->request->state = DONE;
					return ;
				}
				else
				{
					std::cout << "index is : " << file << std::endl;
					newPath = newPath + "/" + file;
					std::cout << "the file path is : " << newPath << std::endl;
					this->handleGet(FILE, newPath);
				}
			}
			else
			{
				if (this->client->location->autoindex == true)
					this->handleGet(DIRE, newPath);
				else
				{
					if (checkError(403))
						this->responseStr = generateError(E403, DEFAULT);
					else
						this->responseStr = generateError(E403, MINE);
					send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
					this->responseSent = 1;
					this->client->request->state = DONE;
					return ;
				}
			}
		} 
	}
    else if (S_ISREG(infos.st_mode))
    {
		std::cout << "IT'S A FILE" << std::endl;
		if (this->client->request->method == "GET")
			this->handleGet(FILE, newPath);
        else if (this->client->request->method == "POST")
			this->handlePost();
		else if (this->client->request->method == "DELETE")
			this->handleDelete(newPath);
		else
		{
			if (checkError(405))
				this->responseStr = generateError(E405, DEFAULT);
			else
				this->responseStr = generateError(E405, MINE);
			send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
			this->responseSent = 1;
			this->client->request->state = DONE;
			return ;
		}
	}
}

std::string	removeBackSlashes(std::string url)
{
	while (url.back() == '/' && url.size() != 1)
		url.pop_back();
	return (url);
}

void	Response::getQuery()
{
	std::string				query;
	size_t					pos;

	pos = this->client->request->url.find('?');
	if (pos != std::string::npos)
	{
		query = this->client->request->url.substr(pos + 1, this->client->request->url.length() - pos + 1);
		this->client->request->query = query;
		this->client->request->url = this->client->request->url.substr(0, pos);
	}
}

void	Response::checkReturn()
{
	if (this->client->location->returned != 0)
	{
		if (this->client->location->returnType == "permanently")
			this->responseStr = "HTTP/1.1 308 Permanent Redirect\r\n"
								"Location: " + this->client->location->returnUrl + "\r\n"
								"Content-Type: text/html\r\n"
								"Content-Length: 0\r\n"
								"Connection: close\r\n\r\n";
		else
			this->responseStr = "HTTP/1.1 307 Temporary Redirect\r\n"
								"Location: " + this->client->location->returnUrl + "\r\n"
								"Content-Type: text/html\r\n"
								"Content-Length: 0\r\n"
								"Connection: close\r\n\r\n";
		send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		this->started = 1;
		this->responseSent = 1;
		this->client->request->state = DONE;
	}
}

void    Response::handleNormalReq()
{
	std::cout << "in handleNormalReq" << std::endl;
	// Parser::lex()->set_input(this->client->request->contentType);
	// this->client->request->contentType = Parser::lex()->next_token(false);


	this->client = &Servme::getCore()->map_clients[this->client_fd];
	if (this->responseSent == 0)
	{
		this->storeMimeTypes();
		this->client->selectServer();
    	this->client = &Servme::getCore()->map_clients[this->client_fd];
		this->getQuery();
		this->client->request->url = removeBackSlashes(this->client->request->url);
    	this->matchLocation(this->client->server->locations);
		std::cout << "matchina m3aa : " << this->client->location->path << std::endl;
		this->checkReturn();
		this->checkAllowedMethods();
		if (this->client->request->url != this->client->location->path)
		{
			if (this->client->location->path != "/")
				this->client->request->url = this->client->request->url.erase(this->client->request->url.find(this->client->location->path), this->client->location->path.length());
			this->client->path = this->client->location->root + this->client->request->url;
		}
		else
			this->client->path = this->client->location->root;
	}
	this->checkPath();
}