#include "../inc/client.hpp"
#include "../inc/macros.hpp"
#include <cstdlib>

Response::Response()
{
	//std::cout << "Response created" << std::endl;
	this->GENERATE_RES = false;
	this->responseSent = 0;
	this->responseStr = "";
	this->body = "";
	this->contentLength = 0;
	this->started = 0;
	this->sendPos = 0;
	this->readPos = 0;
}

Response::~Response()
{
	//std::cout << "Response destroyed" << std::endl;
}

int	Response::checkAccess(std::string path)
{
    std::ifstream file(path.substr(1));
    if (!file.good())
		return (0);
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

	this->client = Servme::getCore()->map_clients[this->client_fd];

	if (this->client == nullptr)
	{
		//std::cout << "why no server" << std::endl;
		return 0;
	}
	if (this->client->location != nullptr)
	{
		root = this->client->location->root;
		if (!this->client->location->error_page.empty())
			member = this->client->location->error_page;
		else
		{
			if (!this->client->server->error_page.empty())
				member = this->client->server->error_page;
		}
	}
	else
	{
		this->client->selectServer();
		if (!this->client->server->error_page.empty())
			member = this->client->server->error_page;
		root = this->client->server->root;
	}
	if (member.empty())
		return 0;
	member.find(error);
	if (member.find(error) != member.end())
	{
		candidates = member[error];
		for (std::vector<std::string>::iterator it = candidates.begin(); it != candidates.end(); it++)
		{
			path = root + "/" + *it;
			if (checkAccess(path))
				return 1;
		}
	}
	return 0;
}

void	Response::storeMimeTypes()
{
	std::ifstream	file("mime.types");
	std::map<std::string, std::string>	mimetypes;

	if (!file.is_open())
		throw std::runtime_error(E500);

	std::string	line;
	while (getline(file, line))
	{
		std::string	extension;
		std::string type;
		Parser::lex()->set_input(line);
		extension = Parser::lex()->next_token(true);
		type = Parser::lex()->next_token(true);
		if (type.back() == '\r' || type.back() == '\n')
			type.pop_back();
		this->contentTypes[extension] = type;
	}
}



void	Response::checkAllowedMethods()
{
	std::vector<std::string>			methods;
	std::vector<std::string>::iterator	iter;

	this->client = Servme::getCore()->map_clients[this->client_fd];

	methods = this->client->location->allowed_methods;
	if (methods.empty() && this->client->request->method == "GET")
		return ;
	for (iter = methods.begin(); iter < methods.end(); iter++)
	{
		if (*iter == this->client->request->method)
			return ;
	}
	throw std::runtime_error(E405);
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


std::vector<Location>	Response::getLocations(std::vector<Location> locations)
{
	std::vector<Location>	candidates;
	std::vector<Location>::iterator	iter;
	this->client = Servme::getCore()->map_clients[this->client_fd];

	iter = locations.begin();
	for (iter = this->client->server->locations.begin(); iter < this->client->server->locations.end(); iter++)
	{
		if (iter->path.back() != '$')
		{
			iter->path = normalizePath(iter->path);
			candidates.push_back(*iter);
		}
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
    return (count);
}

bool	compareFields(const Location& loc1, const Location& loc2)
{
	return (countFields(loc1.path) > countFields(loc2.path));
}

void	Response::matchLocation(std::vector<Location> locations)
{
	std::vector<Location>	candidates;
	std::vector<Location>::iterator	iter;

	this->client = Servme::getCore()->map_clients[this->client_fd];

	candidates = this->getLocations(locations);
	if (!candidates.empty())
	{
		std::sort(candidates.begin(), candidates.end(), compareFields);
		for (iter = candidates.begin(); iter < candidates.end(); iter++)
		{
			if (LocationFound(iter->path, this->client->request->url))
			{
				this->client->location = new Location(*iter);
				return ;
			}
		}
	}
	throw std::runtime_error(E404);
}

void	Response::checkCgi()
{
	std::string	cgiPath = "/cgi-bin/";
	this->client = Servme::getCore()->map_clients[this->client_fd];
	std::string	imaginaryPath = this->client->request->url;
	if (imaginaryPath.find(cgiPath) != std::string::npos)
	{
		this->client->cgiFlag = 1;
		return ;
	}
	else
		this->client->cgiFlag = 0;
}

std::string	Response::getIndex()
{
	std::vector<std::string>::iterator	it;

	for (it = this->client->location->index.begin(); it < this->client->location->index.end(); it++)
	{
		if (access((this->newPath + "/" + (*it)).c_str(), R_OK) == 0)
			return (*it);
	}
	return ("");
}

int	Response::checkReturn()
{
	if (this->client != nullptr)
	{
		if (this->client->location != nullptr)
		{
			if (this->client->location->returned != 0)
			{
				if (this->client->location->returnType == "permanent")
					this->responseStr = "HTTP/1.1 308 Permanent Redirect\r\n"
										"Location: " + this->client->location->returnUrl + "\r\n"
										"Content-Type: text/html\r\n"
										"Content-Length: 0\r\n"
										"Connection: keep-alive\r\n\r\n";
				else
					this->responseStr = "HTTP/1.1 307 Temporary Redirect\r\n"
										"Location: " + this->client->location->returnUrl + "\r\n"
										"Content-Type: text/html\r\n"
										"Content-Length: 0\r\n"
										"Connection: keep-alive\r\n\r\n";
				send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
				this->responseSent = 1;
				this->client->request->state = DONE;
				return (1);
			}
		}
		else
		{
			if (this->client->server != nullptr)
			{
				if (this->client->server->returned != 0)
				{
					if (this->client->server->returnType == "permanent")
						this->responseStr = "HTTP/1.1 308 Permanent Redirect\r\n"
											"Location: " + this->client->server->returnUrl + "\r\n"
											"Content-Type: text/html\r\n"
											"Content-Length: 0\r\n"
											"Connection: keep-alive\r\n\r\n";
					else
						this->responseStr = "HTTP/1.1 307 Temporary Redirect\r\n"
											"Location: " + this->client->server->returnUrl + "\r\n"
											"Content-Type: text/html\r\n"
											"Content-Length: 0\r\n"
											"Connection: keep-alive\r\n\r\n";
					send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
					this->responseSent = 1;
					this->client->request->state = DONE;
					return (1);
				}
			}

		}
	}
	return (0);
}


int		Response::checkResourseType()
{
    struct    stat    infos;

	this->newPath = this->client->path.substr(1);
	if (stat(this->newPath.c_str(), &infos) == -1)
		throw std::runtime_error(E404);
	else if (S_ISDIR(infos.st_mode))
		return (DIRE);
	return (FILE);
}

void	Response::handleDirectory()
{
	if (this->client->request->method == "GET")
		this->handleGet(DIRE);
	else if (this->client->request->method == "POST")
		this->handlePost();
	else if (this->client->request->method == "DELETE")
		throw std::runtime_error(E403);
	else
		throw std::runtime_error(E405);
}

void	Response::handleFile()
{
	if (this->client->request->method == "GET")
		this->handleGet(FILE);
	else if (this->client->request->method == "POST")
		this->handlePost();
	else if (this->client->request->method == "DELETE")
		this->handleDelete();
	else
		throw std::runtime_error(E405);
}

void	Response::getPath()
{
	if (this->client->request->url != this->client->location->path)
	{
		if (this->client->location->path != "/")
			this->client->request->url = this->client->request->url.erase(this->client->request->url.find(this->client->location->path), this->client->location->path.length());
		this->client->path = this->client->location->root + this->client->request->url;
	}
	else
		this->client->path = this->client->location->root;
}

void	Response::parseUrl()
{
	std::string				query;
	size_t					pos;

	pos = std::string::npos;
	if (this->client)
	{
		if (this->client->request)
		{
			if (this->client->request->url != "")
			{
				pos = this->client->request->url.find('?');
				this->client->request->url = normalizePath(this->client->request->url);
			}
		}
	}
	if (pos != std::string::npos)
	{
		query = this->client->request->url.substr(pos + 1, this->client->request->url.length() - pos + 1);
		this->client->request->query = query;
		this->client->request->url = this->client->request->url.substr(0, pos);
	}
	while (this->client->request->url.size() != 1 && this->client->request->url.back() == '/')
		this->client->request->url.pop_back();	
}


void    Response::handleNormalReq()
{
	this->client = Servme::getCore()->map_clients[this->client_fd];
	// try
	// {
		if (this->responseSent == 0)
		{
			if (this->checkReturn())
				return ;
			this->storeMimeTypes();
			this->parseUrl();
			if (this->client->server->locations.empty())
				throw std::runtime_error(E404);
    		this->matchLocation(this->client->server->locations);
			if (this->checkReturn())
				return ;
			if (!this->client->location)
				throw std::runtime_error(E404);
			this->checkAllowedMethods();
			this->getPath();
		}
		if (this->checkResourseType() == FILE)
			this->handleFile();
		else if (this->checkResourseType() == DIRE)
			this->handleDirectory();
		if (this->responseSent == 0)
		{
			send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
			this->responseSent = 1;
		}
	// }
	// catch(const std::exception& e)
	// {
	// 	Parser::lex()->set_input(e.what());
	// 	int	code = atoi(Parser::lex()->next_token(false).c_str());
	// 	if (checkError(code))
	// 		this->responseStr = generateError(e.what(), DEFAULT);
	// 	else
	// 		this->responseStr = generateError(e.what(), MINE);
	// 	send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
	// 	this->responseSent = 1;
	// 	this->client->fd = -1;
	// 	this->client->request->state = DONE;
	// }
}


std::string	Response::parseCookies()
{
	std::multimap<std::string , std::string >::iterator it =  this->client->request->headers.equal_range("Cookie:").first;
	std::multimap<std::string , std::string >::iterator ite =  this->client->request->headers.equal_range("Cookie:").second;
	std::string	cookies;
	for(; it != ite; it++)
	{
		cookies += (*it).second;
	}
	return cookies; 
}