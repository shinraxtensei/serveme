#include "../inc/client.hpp"

void	Response::checkAllowedMethods()
{
	std::vector<std::string>			methods;
	std::vector<std::string>::iterator	iter;

	this->client = &Servme::getCore()->map_clients[this->client_fd];
	methods = this->client->server->allowed_methods;
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

	this->client = &Servme::getCore()->map_clients[this->client_fd];
	// iter = locations.begin();
	// for (iter = this->client->server->locations.begin(); iter < this->client->server->locations.end(); iter++)
	// {
	// 	candidates.push_back(*iter);
	// 	candidates = getNested(candidates, *iter);
	// }
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

void    Response::checkPath()
{
    struct    stat    infos;

    if (stat(this->client->path.c_str(), &infos) == 0)
        std::cout << "File or directory not found" << std::endl;
    if (S_ISDIR(infos.st_mode))
        std::cout << "It's a directory" << std::endl;
    else if (S_ISREG(infos.st_mode))
    {
        std::cout << "It's a file" << std::endl;
        if (access(this->client->path.c_str(), R_OK) == 0)
        {
            // this->handleGet();
        }
        else
            std::cout << "permission denied" << std::endl;
    }
}

void    Response::handleNormalReq()
{
    this->client = &Servme::getCore()->map_clients[this->client_fd];
    this->matchLocation(this->client->server->locations);
	this->client->request->url.substr(client->request->url.find_first_of('/'), client->request->url.size() - client->request->url.find_first_of('/'));
    this->client->path = this->client->location->root + this->client->request->url;
    this->checkPath();
}