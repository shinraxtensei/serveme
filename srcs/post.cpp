#include "../inc/client.hpp"
#include "../inc/macros.hpp"

void	Response::handleMultipart()
{
	std::cout << "in handleMultipart" << std::endl;



	// exit (1);
	this->client = &Servme::getCore()->map_clients[this->client_fd];
	std::multimap<std::string, Multipart_ENV>::iterator	iter;
	for (iter = this->client->request->multipart_env.begin(); iter != this->client->request->multipart_env.end(); iter++)
	{
		std::cout << "wst loop" << std::endl;
		if ((*iter).second.pos < (*iter).second.data.length())
		{
			if ((*iter).second.file_name == "")
			{
				std::map<std::string, std::string>::iterator	i;
				std::string	extension;
				for (i = this->contentTypes.begin(); i != this->contentTypes.end(); i++)
				{
					if ((*i).second.find((*iter).second.content_type) != std::string::npos)
					{
						extension = (*i).first;
						break;
					}
				}
				std::cout << "extension: " << extension << std::endl;
				exit(0);
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
			std::string store;
			this->multipart.open(path, std::ios::binary);
			if (this->multipart.good())
			{
				store = (*iter).second.data.substr((*iter).second.pos, toStore);
				this->multipart << (*iter).second.data;
				this->multipart.close();
				(*iter).second.pos += toStore;
				this->multipart.close();
			}
			else
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
			break ;
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
		// send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		this->responseSent = 1;
		std::cout << "reponse mchat" << std::endl;
	}
}

void	Response::handlePost()
{
	std::cout << "in handlePost" << std::endl;
	this->client = &Servme::getCore()->map_clients[this->client_fd];
	if (this->client->request->bodyType == MULTIPART)
		this->handleMultipart();
	else
	{
		std::cout << "le body is : " << this->client->request->bodyString << std::endl;
		if (this->readPos < this->client->request->bodyString.length())
		{
			if (this->started == 0)
			{
				this->client->request->contentType = "text/html";
				std::cout << "content type is : " << this->client->request->contentType << std::endl;
				std::map<std::string, std::string>::iterator	i;
				std::string	extension;
				for (i = this->contentTypes.begin(); i != this->contentTypes.end(); i++)
				{
					std::cout << "looking for types" << std::endl;
					if (this->client->request->contentType == (*i).second)
					{
						std::cout << "lvalue hiya " << (*i).first << std::endl;
						extension = (*i).first;
						break ;
					}
				}
				if (i == this->contentTypes.end())
					extension = "txt";
				std::cout << "extension is : " << extension << std::endl;
				std::string	filename = "random." + extension;
				std::string	path = "upload/" + filename;
				file1.open(path, std::ios::binary);
				if (!file1.good())
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
				this->started = 1;
			}
			else
			{
				int	toStore;
				if (this->client->request->bodyString.length() - this->readPos > 1024)
					toStore = 1024;
				else
					toStore = this->client->request->bodyString.length() - this->readPos;
				std::string store = this->client->request->bodyString.substr(this->readPos, toStore);
				file1 << this->client->request->bodyString;
				file1.flush();
				this->readPos += toStore;
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
		if (this->started == 1 && this->readPos == this->client->request->bodyString.length())
		{
			this->client->request->state = DONE;
			file1.close();
		}
	}
}
