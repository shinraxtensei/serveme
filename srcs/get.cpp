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
    this->responseStr = "";
	int	buffer_size = 5000;
    char buffer[buffer_size];
	int i = 0;
	while (file)
	{
		std::cout << "ba9i kanseft" << std::endl;
        file.read(buffer, buffer_size);
        auto count = file.gcount();
        if (count > 0) {
            std::ostringstream chunk_header;
            chunk_header << std::hex << count << "\r\n";
            this->responseStr = chunk_header.str();
            send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
            send(this->client_fd, buffer, count, 0);
            send(this->client_fd, "\r\n", 2, 0);
			std::cout << "ha : " << i << std::endl;
			i++;
        }
    }
	std::cout << "safi rah salina" << std::endl;
	send(this->client_fd, "0\r\n\r\n", 5, 0);
    // this->responseStr << "0\r\n\r\n";	
}

void	Response::sendFile(std::string newPath)
{
	std::ifstream file(newPath.c_str());
	if (!file.good())
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
		"Content-Length: 10589111\r\n"
		// "Connection: keep-alive\r\n"
		"Transfer-Encoding: chunked\r\n\r\n";
		send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		this->sendChunked(file);
		this->responseSent = 1;
		std::cout << "response sent" << std::endl;
}