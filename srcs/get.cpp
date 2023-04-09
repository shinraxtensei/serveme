#include "../inc/client.hpp"
#include "../inc/macros.hpp"

void	Response::listDirectory(std::string	newPath, DIR *dir)
{
		std::string	body = "<html>\n<head></head>\n<body>\n<h1>Index of " + newPath + "</h1>\n"
			"<table style=\"width: 50%\">\n"
			"<tr><td>Name</td><td>Last Modified</td><td>Size</td></tr>\n";
			struct	dirent *dirent;
			while ((dirent = readdir(dir)) != NULL)
			{
				std::string	filename = dirent->d_name;
				if (filename == "." || filename == "..")
					continue ;
				std::string	filePath = "/" + newPath + "/" + filename;
				std::cout << filePath << std::endl;
				struct stat	fileinfo;
				if (stat(filePath.c_str(), &fileinfo) < 0)
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
				std::string	size;
				if (S_ISREG(fileinfo.st_mode))
					size = std::to_string(fileinfo.st_size);
				else
					size = "-";
				time_t last_modified_timestamp = fileinfo.st_mtime;
				std::string	time = std::string(ctime(&last_modified_timestamp));
				body = body + "<tr><td><a href=\"" + filename + "\">" + "</a></td><td>" + time + "</td><td>" + size + "</td></tr>\n";		
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
			closedir(dir);
			send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
			this->responseSent = 1;
			this->client->request->state = DONE;
			return ;
}

int getpollfd(pollfd clientPollfd)
{
	for (size_t i = 0; i < Servme::getCore()->pollFds.size() ; i++)
	{
		if (Servme::getCore()->pollFds[i].fd == clientPollfd.fd)
			return i;
	}
	return (-1);
}

void	Response::sendFile(std::string newPath)
{
	std::cout << "in sendFile" << std::endl;
	if (this->started == 0)
	{
		this->file.open(newPath.c_str(), std::ios::binary);
		if (!this->file.good())
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
		std::string	extension;
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
		std::cout << "content type : " << contentType << std::endl;
		this->responseStr =
    	"HTTP/1.1 200 OK\r\n"
		"Content-Type: "
		+ contentType + "\r\n"
		"Content-Length: "
		+ std::to_string(this->contentLength) + "\r\n"
		"Connection: keep-alive\r\n\r\n";
		send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		this->started = 1;
		return;
	}
	if (this->sendPos < this->contentLength)
	{
		int	size;
		if (this->contentLength - this->sendPos > 1024)
			size = 1024;
		else
			size = this->contentLength - this->sendPos;
		char	buffer[size];
		this->file.read(buffer, sizeof(buffer));
		this->sendPos += size;
		send(this->client_fd, buffer, sizeof(buffer), 0);
	}
	if (this->sendPos == this->body.length())
	{
		this->responseSent = 1;
		this->client->request->state = DONE;
	}
}


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
		{
			//throw exception
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

void	Response::sendFile()
{
	if (this->started == 0)
	{
	this->fileRead.open(this->newPath.c_str(), std::ios::binary);
	if (!this->fileRead.good())
	{
		//throw exception
		if (checkError(500))
			this->responseStr = generateError(E500, DEFAULT);
		else
			this->responseStr = generateError(E500, MINE);
		send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		this->responseSent = 1;
		this->client->request->state = DONE;
		return ;
	}
	std::string	extension;
	std::string	contentType;
	std::string::size_type dotIndex = this->newPath.rfind('.');
	if (dotIndex != std::string::npos)
	{
		extension = this->newPath.substr(dotIndex + 1);
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
	"Content-Length: "
	+ std::to_string(this->contentLength) + "\r\n"
	"Connection: keep-alive\r\n\r\n";
	send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
	this->started = 1;
	return;
	}
	if (this->sendPos == this->contentLength)
	{
		this->responseSent = 1;
		this->client->request->state = DONE;
	}
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
	//do not send here
}