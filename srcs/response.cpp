#include "../inc/client.hpp"


Response::Response()
{
	this->GENERATE_RES = false;
}

void	Response::checkAllowedMethods()
{
	std::vector<std::string>			methods;
	std::vector<std::string>::iterator	iter;

	std::cout << "htal hna mzyan" << std::endl;
	this->client = &Servme::getCore()->map_clients[this->client_fd];
	std::cout << "htal hnaa mzyan" << std::endl;
	methods = this->client->server->allowed_methods;
	std::cout << "htal hnaaa mzyan" << std::endl;
	if (methods.size() == 0 && this->client->request->method == "GET")
		return ;
	for (iter = methods.begin(); iter < methods.end(); iter++)
	{
		if (*iter == this->client->request->method)
			break ;
	}
	if (iter == methods.end())
		std::cout << "method not allowed" << std::endl;
}

int		LocationFound(std::string locationPaths, std::string	path)
{
	// std::cout << "locationPaths: " << locationPaths << std::endl;
	std::cout << "in location found" << std::endl;
	if (locationPaths == path)
			return (1) ;
	while (!path.empty() && path != "/")
	{
		if (locationPaths == path)
			return (1) ;
		path = path.substr(0, path.find_last_of("/"));
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

void	Response::matchLocation(std::vector<Location> locations)
{
	std::vector<Location>	candidates;
	std::vector<Location>::iterator	iter;

	std::cout << "matching location" << std::endl;
	this->client = &Servme::getCore()->map_clients[this->client_fd];
	candidates = this->getLocations(locations);
	for (iter = candidates.begin(); iter < candidates.end(); iter++)
	{
		if (LocationFound(iter->path, this->client->request->url))
		{
			this->client->location = new Location(*iter);
			std::cout << "location found" << std::endl;
			return ;
		}
	}
	std::cout << "No location found" << std::endl;
}

void	Response::checkCgi()
{
	std::string	cgiPath = "/cgi-bin/";
	this->client = &Servme::getCore()->map_clients[this->client_fd];
	std::string	imaginaryPath = this->client->request->url;
	if (imaginaryPath.find(cgiPath) != std::string::npos)
	{
		std::cout << "cgi request" << std::endl;
		this->client->cgiFlag = 1;
		return ;
	}
	else
	{
		std::cout << "not a cgi request" << std::endl;
		this->client->cgiFlag = 0;
	}
}

void	Response::handleGet(int type, std::string newPath)
{
	// struct    stat    infos;

	// if (type == DIR)
	// {
	// 	if (!this->client->location->index.empty())
	// 	{
	// 		std::vector <std::string>::iterator	iter;
	// 		for (iter = this->client->location->index.begin(); iter < this->client->location->index.end(); iter++)
	// 		{
	// 			if (stat((this->client->path + (*iter)).c_str(), &infos) == 0)
	// 			{
	// 				this->client->path = this->client->path + (*iter);
	// 				break ;
	// 			}
	// 		}
	// 	}
	// 	// this->client->path = this->client->path + this->client->location->index;
	// }
	std::cout << "handling file" << std::endl;
	if (type == FILE)
	{
		std::cout << "hnaa" << std::endl;
        if (access(newPath.c_str(), R_OK) == 0)
        {
			std::ifstream file(newPath.c_str());
			if (!file.good()) {
    			std::cerr << "Error opening file\n";
    			return ;
			}
    		std::stringstream buffer;
    		buffer << file.rdbuf();
    		this->body = buffer.str();
    		file.close();
			this->responseStr =
    			"HTTP/1.1 200 OK\r\n"
    			"Content-Type: text/html\r\n"
    			"Content-Length: \r\n"
				"Connection: close\r\n\r\n" + this->body;
			std::cout << "----------------------------" << std::endl;
			std::cout << "response: " << this->responseStr << std::endl;
			std::cout << "----------------------------" << std::endl;
			send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		}
		else
			std::cout << "access denied" << std::endl;
	}
}

std::string	Response::getIndex(std::string newPath)
{
	std::vector<std::string>::iterator	it;

	for (it = this->client->location->index.begin(); it < this->client->location->index.end(); it++)
	{
		// std::cout << "kan9llb" << std::endl;
		std::cout << *it << std::endl;
		std::cout << "file path : " << this->client->path + "/" + (*it) << std::endl;
		if (access((newPath + "/" + (*it)).c_str(), R_OK) == 0)
		{
			return (*it);
		}
		else
			std::cout << "ma3ndekch l7e99" << std::endl;
	}
	return ("");
}

// void	Response::listDirectory()
// {
// 	DIR *dir;
// 	struct dirent *ent;
// 	std::string	newPath = this->client->path.substr(this->client->path.find_first_of('/') + 1, this->client->path.length() - this->client->path.find_first_of('/') + 1);
// 	std::cout << "newPath: " << newPath << std::endl;
// 	std::string	index = this->getIndex(newPath);
// 	std::cout << "index: " << index << std::endl;
// 	if (index != "")
// 	{
// 		std::cout << "hnaa" << std::endl;
// 		this->handleGet(FILE, newPath + "/" + index);
// 		return ;
// 	}
// 	if ((dir = opendir(newPath.c_str())) != NULL)
// 	{
// 		while ((ent = readdir(dir)) != NULL)
// 		{
// 			std::cout << ent->d_name << std::endl;
// 			this->body += ent->d_name;
// 			this->body += "
// }

void    Response::checkPath()
{
    struct    stat    infos;

	std::cout << "in check path" << std::endl;
	std::cout << "path: " << this->client->path << std::endl;
	std::string	newPath = this->client->path.substr(this->client->path.find_first_of('/') + 1, this->client->path.length() - this->client->path.find_first_of('/') + 1);
	std::cout << "newPath: " << newPath << std::endl;
    if (stat(newPath.c_str(), &infos) == 0)
        std::cout << "File found" << std::endl;
	else
		std::cout << "File not found" << std::endl;
    if (S_ISDIR(infos.st_mode))
	{
        std::cout << "It's a directory" << std::endl;
		if (!(this->client->location->index.empty()))
		{
			std::string file = this->getIndex(newPath);
			std::cout << "file: " << file << std::endl;
			if(!file.empty())
			{
				std::cout << "index found" << std::endl;
				// this->client->path = this->client->path + "/" + file;
				newPath = newPath + "/" + file;
				this->handleGet(FILE, newPath);
			}
			else
				std::cout << "Forbidden" << std::endl;
		} 
		else
		{
			if (this->client->location->autoindex == true)
			{
				std::cout << "autoindex" << std::endl;
				this->handleGet(DIR, newPath);
			}
		}
	}
    else if (S_ISREG(infos.st_mode))
    {
        std::cout << "It's a file" << std::endl;
		if (this->client->request->method == "GET")
			this->handleGet(FILE, newPath);
        else
            std::cout << "Forbidden " << std::endl;
    }
}

std::string	removeBackSlashes(std::string url)
{
	if (url.size() != 1)
	{
		while (url.back() == '/')
			url.pop_back();
	}
	return (url);
}

void    Response::handleNormalReq()
{
	std::cout << "in handle normal req" << std::endl;
    this->client = &Servme::getCore()->map_clients[this->client_fd];
	this->client->request->url = removeBackSlashes(this->client->request->url);
    this->matchLocation(this->client->server->locations);
	if (this->client->request->url != this->client->location->path)
	{
		this->client->request->url = this->client->request->url.erase(this->client->request->url.find(this->client->location->path), this->client->location->path.length());
		this->client->path = this->client->location->root + this->client->request->url;
	}
	else
		this->client->path = this->client->location->root;
    this->checkPath();
}