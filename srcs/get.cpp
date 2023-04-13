#include "../inc/client.hpp"
#include "../inc/macros.hpp"


// int getpollfd(pollfd clientPollfd)
// {
// 	for (size_t i = 0; i < Servme::getCore()->pollFds.size() ; i++)
// 	{
// 		if (Servme::getCore()->pollFds[i].fd == clientPollfd.fd)
// 			return i;
// 	}
// 	return (-1);
// }

void	Response::handleGet(int type)
{
	if (type == FILE)
	{
		struct stat	infos;
		stat(this->newPath.c_str(), &infos);
		this->contentLength = infos.st_size;
		if (access(this->newPath.c_str(), R_OK) == 0)
			this->sendFile();
		else
			throw std::runtime_error(E403);
	}
	else if (type == DIRE)
	{
		if (this->client->location->index[0] != "")
		{
			std::string index = this->getIndex();
			if (index != "")
			{
				this->newPath = this->newPath + "/" + index;
				this->handleGet(FILE);
			}
			else
				throw std::runtime_error(E403);
		}
		else
		{
			if (this->client->location->autoindex == true)
				this->sendDirectory();
			else
				throw std::runtime_error(E404);
		}
	}
}

void	Response::sendDirectory()
{
	if (this->responseSent == 0)
	{
    	DIR	*dir = opendir(this->client->path.substr(1).c_str());
		if (dir == NULL)
			throw std::runtime_error(E500);
		// std::cout << "t7eeeeel" << std::endl;
		// exit (1);
		std::string	body = "<html>\n<head></head>\n<body>\n<h1>Index of " + newPath + "</h1>\n"
			"<table style=\"width: 50%\">\n"
			"<tr><td>Name</td><td>Last Modified</td><td>Size</td></tr>\n";
		struct	dirent	*dirent;
		struct	stat	infos;
		while ((dirent = readdir(dir)) != NULL)
		{
			std::string filename = dirent->d_name;
			if (filename[0] != '.')
			{
				std::string path = this->newPath + "/" + filename;
				if (stat(path.c_str(), &infos) < 0)
				{
					std::cout << "hna trat chi haja" << std::endl;
					exit (1);
					throw std::runtime_error(E500);
				}
				std::string size;
				if (S_ISDIR(infos.st_mode))
					size = "-";
				else
					size = std::to_string(infos.st_size);
				std::string lastModified = ctime(&infos.st_mtime);
				body += "<tr><td><a href=\"" + filename + "\">" + filename + "</a></td><td>" + lastModified + "</td><td>" + size + "</td></tr>\n";
			}
		}
		body += "</table>\n</body>\n</html>";
		this->body = body;
		std::stringstream	ss;
		ss << this->body.size();
		closedir(dir);
		this->responseStr = 
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Accept-Ranges: none\r\n"
			"Content-Length: " + ss.str() + "\r\n"
			"Connection: close\r\n\r\n";
		return ;
	}
	if (this->responseSent == 1 && this->sendPos == this->body.size())
		this->client->request->state = DONE;
	int toSend;
	if (this->body.size() - this->sendPos > 1024)
		toSend = 1024;
	else
		toSend = this->body.size() - this->sendPos;
	std::string toSendStr = this->body.substr(this->sendPos, toSend);
	this->sendPos += toSend;
	send(this->client_fd, toSendStr.c_str(), toSendStr.length(), 0);
}

void	Response::sendFile()
{
	if (this->responseSent == 0)
	{
		this->fileRead.open(this->newPath.c_str(), std::ios::binary);
		if (!this->fileRead.good())
			throw std::runtime_error(E500);
		std::string	extension;
		std::string	contentType;
		std::string::size_type dotIndex = this->newPath.rfind('.');
		if (dotIndex != std::string::npos)
		{
			extension = this->newPath.substr(dotIndex + 1);
			contentType = this->contentTypes[extension];
			if (extension == "php")
				contentType = "text/html";
			if (contentType == "")
				contentType = "text/plain";
		}
		else
			contentType = "text/plain";
		this->responseStr =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: "
		+ contentType + "\r\n"
		// "Accept-Ranges: none\r\n"
		"Content-Length: "
		+ std::to_string(this->contentLength) + "\r\n"
		"Connection: close\r\n\r\n";
		return ;
	}
	if (this->responseSent == 1 && this->sendPos == this->contentLength)
		this->client->request->state = DONE;
	if (this->sendPos < this->contentLength)
	{
		int	size;
		if (this->contentLength - this->sendPos > 1024)
			size = 1024;
		else
			size = this->contentLength - this->sendPos;
		char	buffer[size];
		this->fileRead.read(buffer, sizeof(buffer));
		this->sendPos += size;
		send(this->client_fd, buffer, sizeof(buffer), 0);
	}
}