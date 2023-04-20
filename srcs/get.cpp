#include "../inc/client.hpp"
#include "../inc/macros.hpp"

void Response::handleGet(int type)
{
	if (type == FILE)
	{
		struct stat infos;
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

void Response::sendDirectory()
{
	if (this->responseSent == 0)
	{
		DIR *dir = opendir(this->client->path.substr(1).c_str());
		if (dir == NULL)
			throw std::runtime_error(E500);
		std::string body = "<html>\n<head></head>\n<body>\n<h1>Index of " + newPath + "</h1>\n"
																					  "<table style=\"width: 50%\">\n"
																					  "<tr><td>Name</td><td>Last Modified</td><td>Size</td></tr>\n";
		struct dirent *dirent;
		struct stat infos;
		while ((dirent = readdir(dir)) != NULL)
		{
			std::string filename = dirent->d_name;
			if (filename[0] != '.')
			{
				std::string path = this->newPath + "/" + filename;
				if (stat(path.c_str(), &infos) < 0)
					throw std::runtime_error(E500);
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
		std::stringstream ss;
		ss << this->body.size();
		closedir(dir);
		this->responseStr =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Accept-Ranges: none\r\n"
			"Content-Length: " +
			ss.str() + "\r\n"
			"Connection: keep-alive\r\n\r\n";
		return;
	}
	if (this->responseSent == 1 && this->sendPos == this->body.size())
	{
		this->client->request->state = DONE;
		return ;
	}
	int toSend;
	if (this->body.size() - this->sendPos > 1024)
		toSend = 1024;
	else
		toSend = this->body.size() - this->sendPos;
	this->responseStr = this->body.substr(this->sendPos, toSend);
	int sent = send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
	if (sent == -1 || sent == 0)
		throw std::runtime_error(E500);
	this->sendPos += sent;
}

void Response::sendFile()
{
	if (this->responseSent == 0)
	{
		this->fileRead.open(this->newPath.c_str(), std::ios::binary);
		if (!this->fileRead.good())
			throw std::runtime_error(E500);
		std::string extension;
		std::string contentType;
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
			"Content-Type: " +
			contentType + "\r\n"
			"Accept-Ranges: none\r\n"
			"Content-Length: " +
			std::to_string(this->contentLength) + "\r\n"
			"Connection: keep-alive\r\n\r\n";
		return;
	}
	if (this->responseSent == 1 && this->sendPos == this->contentLength)
	{
		this->fileRead.close();
		this->client->request->state = DONE;
	}
	if (this->sendPos < this->contentLength)
	{
		int size;
		if (this->contentLength - this->sendPos > 1024)
			size = 1024;
		else
			size = this->contentLength - this->sendPos;
		char	buffer[size];
		this->fileRead.read(buffer, size);
		int sent = send(this->client_fd, buffer, sizeof(buffer), 0);
		if (sent == -1)
			return ;
		this->sendPos += sent;
	}
}