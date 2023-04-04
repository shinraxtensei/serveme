#include "../inc/client.hpp"
#include "../inc/macros.hpp"

void	Response::handleMultipart()
{
	std::multimap<std::string, Multipart_ENV>::iterator	iter;

	for (iter = this->client->request->multipart_env.begin(); iter != this->client->request->multipart_env.end(); iter++)
	{
		if ((*iter).second.pos != (*iter).second.data.length())
		{
			if ((*iter).second.file_name == "")
			{
				std::map<std::string, std::string>::iterator	i;
				std::string	extension;
				for (i = this->contentTypes.begin(); i != this->contentTypes.end(); i++)
				{
					if ((*i).second == (*iter).second.content_type)
					{
						extension = (*i).first;
						break;
					}
				}
				if (i == this->contentTypes.end())
					extension = "txt";
				(*iter).second.file_name = "random." + extension;
			}
			int	toStore;
			if ((*iter).second.data.length() - (*iter).second.pos > 1024)
				toStore = 1024;
			else
				toStore = (*iter).second.data.length() - (*iter).second.pos;
			std::string path = "upload/" + (*iter).second.file_name;
			std::ofstream	file(path);
			if (!file.good())
			{
				std::ofstream	upload(path);
				file << (*iter).second.data;
				file.close();
			}
			else
			{
				this->responseStr = generateError(E500);
				send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
				this->responseSent = 1;
				this->client->request->state = DONE;
				return ;
			}
			(*iter).second.pos += toStore;
		}
	}
	if (this->responseSent == 0)
	{
		this->body = "<html><head></head><body><h1>KOULCHI NADI AWLDI</h1></body></html>";
		std::stringstream ss;
		ss << this->body.length();
		this->responseStr = "HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length:" + ss.str() + "\r\n\r\n" + this->body;
		send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		this->responseSent = 1;
	}
}

void	Response::handlePost()
{
	if (this->client->request->bodyType == MULTIPART)
		this->handleMultipart();
	else
	{
		std::map<std::string, std::string>::iterator	i;
		std::string	extension;
		for (i = this->contentTypes.begin(); i != this->contentTypes.end(); i++)
		{
			if (this->client->request->contentType == (*i).second)
			{
				extension = (*i).first;
				break;
			}
		}
		if (i == this->contentTypes.end())
			extension = "txt";
		std::string	filename = "random." + extension;
		std::string	path = "upload/" + filename;
		std::ofstream	file(path);
		if (!file.good())
		{
			this->responseStr = generateError(E500);
			send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
			this->responseSent = 1;
			this->client->request->state = DONE;
			return ;
		}
		else
		{
			int	toStore;
			if (this->client->request->bodyString.length() - this->readPos > 1024)
				toStore = 1024;
			else
				toStore = this->client->request->bodyString.length() - this->readPos;
			std::string store = this->client->request->bodyString.substr(this->readPos, toStore);
			file << this->client->request->bodyString;
			file.close();
			if (this->responseSent == 0)
			{
				this->body = "<html><head></head><body><h1>KOULCHI NADI AWLDI</h1></body></html>";
				std::stringstream ss;
				ss << this->body.length();
				this->responseStr = "HTTP/1.1 200 OK\r\n"
						"Content-Type: text/html\r\n"
						"Content-Length:" + ss.str() + " \r\n"
						"Connection: close\r\n\r\n" + this->body;
				send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
				this->responseSent = 1;
			}
		}
	}
}