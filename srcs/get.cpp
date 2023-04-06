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
					this->responseStr = generateError(E500);
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

void	Response::sendChunked(std::ifstream &file)
{
	std::cout << "in sendChunked" << std::endl;
    this->responseStr = "";
	int	buffer_size = 1024;
    char buffer[buffer_size];
	int i = 0;
	while (file)
	{
		std::cout << "ba9i kanseft" << std::endl;
        file.read(buffer, buffer_size);
        int count = file.gcount();
        if (count > 0) {
            std::ostringstream chunk_header;
			std::cout << "chunk size in decimal : " << count << std::endl;
            chunk_header << std::hex << count << "\r\n";
			std::cout << "chunk size in hexa : " << chunk_header.str() << std::endl;
            this->responseStr = chunk_header.str();
			std::cout << "response str : " << this->responseStr << std::endl;
            if (send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0) == -1)
				std::cout << "error in send" << std::endl;
			std::cout << "size macha" << std::endl;
            if (send(this->client_fd, buffer, count, 0) == -1)
				std::cout << "error in send" << std::endl;
			std::cout << "buffer macha" << std::endl;
            if (send(this->client_fd, "\r\n", 2, 0) == -1)
				std::cout << "error in send" << std::endl;
			std::cout << "ha : " << i << std::endl;
			i++;
        }
    }
	std::cout << "safi rah salina" << std::endl;
	send(this->client_fd, "0\r\n\r\n", 5, 0);
    // this->responseStr << "0\r\n\r\n";	
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
		std::cout << "hna mzyan" << std::endl;
		this->file.open(newPath.c_str(), std::ios::binary);
		std::cout << "hna mzyan" << std::endl;
		if (!this->file.good())
		{
			this->responseStr = generateError(E500);
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
		// std::cout << "first line + headers : " << std::endl << this->responseStr << std::endl;
		send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		this->started = 1;
	}
	if (this->sendPos < this->contentLength)
	{
		std::cout << "ba9i masalinach" << std::endl;
		int	size;
		if (this->contentLength - this->sendPos > 1024)
		{
			std::cout << "dkhlna lhna" << std::endl;
			size = 1024;
		}
		else
			size = this->contentLength - this->sendPos;
		char	buffer[size];
		std::cout << "before reading" << std::endl;
		this->file.read(buffer, sizeof(buffer));
		this->sendPos += size;
		send(this->client_fd, buffer, sizeof(buffer), 0);
		// this->body = std::string(buffer);
		// std::cout << "le body is this->body : " << this->body << std::endl;
	}
	if (this->sendPos == this->body.length())
	{
		this->responseSent = 1;
		this->client->request->state = DONE;
	}
	std::cout << "7na hna" << std::endl;
	std::cout << "request state : " << this->client->request->state << std::endl;
}
