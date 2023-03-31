#include "../inc/client.hpp"
#include "../inc/macros.hpp"


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
		this->responseStr = generateError(E405);
		send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		exit (1);
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

	std::cout << "matching location" << std::endl;
	this->client = &Servme::getCore()->map_clients[this->client_fd];
	candidates = this->getLocations(locations);
	if (candidates.empty())
	{
		std::cout << "no location found" << std::endl;
		this->responseStr = generateError(E404);
		send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		exit (1);
	}
	std::sort(candidates.begin(), candidates.end(), compareFields);
	for (iter = candidates.begin(); iter < candidates.end(); iter++)
	{
		if (LocationFound(iter->path, this->client->request->url))
		{
			std::cout << "location found" << std::endl;
			this->client->location = new Location(*iter);
			return ;
		}
	}
	this->responseStr = generateError(E404);
	send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
	exit (1);
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
			this->responseStr = generateError(E500);
			send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
			exit (1);
		}
		std::string	body = 
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
            	body += "<tr><td><a href=\"";
            	body += filename;
            	body += "\">";
            	body += filename;
            	body += "</a></td><td>";
				body += time;
				body += "</td><td>";
            	body += size;
            	body += "</td></tr>\n";				
			}
			body += "</table>\n"
			"</body>\n"
			"</html>\n";
			this->body = body;
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
			if (!file.good())
			{
				this->responseStr = generateError(E500);
				send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
				exit (1);
			}
    		std::stringstream buffer;
    		buffer << file.rdbuf();
    		this->body = buffer.str();
    		file.close();
			std::string	extension;
			std::string inter;
			std::string	contentType;
			std::string::size_type dotIndex = newPath.rfind('.');
    		if (dotIndex != std::string::npos)
			{
        		extension = newPath.substr(dotIndex + 1);
				contentType = this->contentTypes[extension];
				if (contentType == "")
					contentType = "text/plain";
			}
			else
				contentType = "text/plain";
			this->responseStr =
    			"HTTP/1.1 200 OK\r\n"
    			"Content-Type: "
				 + contentType + "\r\n"
    			"Content-Length: \r\n"
				"Connection: close\r\n\r\n" + this->body;
			std::cout << "----------------------------" << std::endl;
			std::cout << "response: " << this->responseStr << std::endl;
			std::cout << "----------------------------" << std::endl;
			send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		}
		else
		{
			this->responseStr = generateError(E403);
			send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
			exit (1);
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
	this->responseStr = generateError(E404);
	std::cout << "-----------------------------" << std::endl;
	std::cout << "Response : " << std::endl;
	std::cout << this->responseStr << std::endl;
	send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
	exit (1);
}


void    Response::checkPath()
{
    struct    stat    infos;

	std::cout << "lpath houwaaaaaa : " << this->client->path << std::endl;
	std::string	newPath = this->client->path.substr(this->client->path.find_first_of('/') + 1, this->client->path.length() - this->client->path.find_first_of('/') + 1);
	std::cout << "new Path : " << newPath << std::endl;
    if (stat(newPath.c_str(), &infos) == 0)
        std::cout << "File found " << this->client->location->path.c_str() << std::endl;
	else
	{
		this->responseStr = generateError(E404);
		send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		exit (1);
	}
	std::cout << "new Path : " << newPath << std::endl;
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
				this->responseStr = generateError(E404);
				send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
				exit (1);
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
			if (this->client->location->autoindex == true)
			{
				std::cout << "autoindex" << std::endl;
				this->handleGet(DIRE, newPath);
			}
		}
	}
    else if (S_ISREG(infos.st_mode))
    {
        std::cout << "It's a file" << std::endl;
		if (this->client->request->method == "GET")
			this->handleGet(FILE, newPath);
        else if (this->client->request->method == "POST")
			this->handlePost();
		else if (this->client->request->method == "DELETE")
			this->handleDelete();
		else
		{
			this->responseStr = generateError(E405);
			send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
			exit (1);
		}
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
	this->storeMimeTypes();
	std::cout << "in handle normal req" << std::endl;
    this->client = &Servme::getCore()->map_clients[this->client_fd];
	this->client->request->url = removeBackSlashes(this->client->request->url);
    this->matchLocation(this->client->server->locations);
	this->checkAllowedMethods();
	if (this->client->request->url != this->client->location->path)
	{
		std::cout << "machi bhal bhal" << std::endl;
		if (this->client->location->path != "/")
			this->client->request->url = this->client->request->url.erase(this->client->request->url.find(this->client->location->path), this->client->location->path.length());
		std::cout << "khouna bgha : " << this->client->request->url << std::endl;;
		this->client->path = this->client->location->root + this->client->request->url;
		std::cout << "nmchiw n9llbo 3la : " << this->client->path << std::endl;
	}
	else
		this->client->path = this->client->location->root;
    this->checkPath();
}