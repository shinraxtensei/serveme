#include "../inc/client.hpp"


Response::Response()
{
	this->GENERATE_RES = false;
}

Response::~Response()
{
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
	this->responseStr = generateError(E404);
	send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);

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
	if (type == DIRE)
	{
		std::cout << "handling directory listing" << std::endl;
    	DIR	*dir = opendir(this->client->path.c_str());
		if (dir == NULL)
		{
			std::cout << "error opening directory";
			return ;
		}
		std::string	html = 
			"<html>\n"
			"<head><title>Index of " + newPath + "</title></head>\n"
			"<body>\n"
			"<h1>Index of " + newPath + "</h1>\n"
			"<table style=\"width: 50%\">\n"
			"<tr><td>Name</td><td>Last Modified</td><td>Size</td></tr>\n";
			struct	dirent *dirent;
			while ((dirent = readdir(dir)) != NULL)
			{
				std::cout << "stuck hna" << std::endl;
				std::string	filename = dirent->d_name;
				if (filename == "." || filename == "..")
					continue ;
				// std::string statPath = 
				std::string	filePath = "/" + newPath + "/" + filename;
				std::cout << filePath << std::endl;
				struct stat	fileinfo;
				if (stat(filePath.c_str(), &fileinfo) < 0)
				{
					std::cout << "stat problem" << std::endl;
					continue ;
				}
				std::string	size;
				if (S_ISREG(fileinfo.st_mode))
					size = std::to_string(fileinfo.st_size);
				else
					size = "-";
				time_t last_modified_timestamp = fileinfo.st_mtime;
				std::string	time = std::string(ctime(&last_modified_timestamp));
            	html += "<tr><td><a href=\"";
            	html += filename;
            	html += "\">";
            	html += filename;
            	html += "</a></td><td>";
				html += time;
				html += "</td><td>";
            	html += size;
            	html += "</td></tr>\n";				
			}
			html += "</table>\n"
			"</body>\n"
			"</html>\n";
			this->body = html;
			std::stringstream ss;
    		ss << this->body.size();
			this->responseStr =
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length:" + ss.str() + " \r\n"
				"Connection: close\r\n\r\n" + this->body;
			std::cout << "----------------------------" << std::endl;
			std::cout << this->responseStr << std::endl;
			std::cout << "----------------------------" << std::endl;
			closedir(dir);
			send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
	}
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

	std::cout << "in check path" << std::endl;
	std::cout << "path: " << this->client->path << std::endl;
    if (stat(this->client->location->path.c_str(), &infos) == 0)
        std::cout << "File found" << std::endl;
	std::string	newPath = this->client->path.substr(this->client->path.find_first_of('/') + 1, this->client->path.length() - this->client->path.find_first_of('/') + 1);
	std::cout << "newPath: " << newPath << std::endl;
    if (S_ISDIR(infos.st_mode))
	{
        std::cout << "It's a directory" << std::endl;
		if ((this->client->location->index[0] != ""))
		{
			std::cout << "index is here" << std::endl;
			std::string	file = this->getIndex(newPath);
			std::cout << "file: " << file << std::endl;
			if (file == "")
			{
				std::cout << "Forbidden" << std::endl;
				return ;
			}
			else
			{
				std::cout << "index found" << std::endl;
				newPath = newPath + "/" + file;
				this->handleGet(FILE, newPath);
			}
		} 
		else
		{
			// if (this->client->location->autoindex == true)
			// {
				std::cout << "autoindex" << std::endl;
				this->handleGet(DIRE, newPath);
			// }
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
	if (this->client->server->locations.empty())
		return ;
	if (this->client->request->url != this->client->location->path)
	{
		this->client->request->url = this->client->request->url.erase(this->client->request->url.find(this->client->location->path), this->client->location->path.length());
		this->client->path = this->client->location->root + this->client->request->url;
	}
	else
		this->client->path = this->client->location->root;
    this->checkPath();
}