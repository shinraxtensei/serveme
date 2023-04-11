#include "../inc/client.hpp"
#include "../inc/macros.hpp"

void	Response::handlePost()
{
	if (this->client->request->bodyType == MULTIPART)
		this->handleMultipart();
	else
	{
		this->handleNormalBody();
	}
}

void	Response::handleMultipart()
{
	std::multimap<std::string, Multipart_ENV>::iterator	it;
	for (it = this->client->request->multipart_env.begin(); it != this->client->request->multipart_env.end(); it++)
	{
		if ((*it).second.pos < (*it).second.data.length())
		{
			if ((*it).second.file_name == "")
			{
				std::map<std::string, std::string>::iterator	i;
				std::string	extension;
				for (i = this->contentTypes.begin(); i != this->contentTypes.end(); i++)
				{
					if ((*it).second.content_type == (*i).second)
					{
						extension = (*i).first;
						break ;
					}
				}
				if (i == this->contentTypes.end())
					extension = "txt";
				(*it).second.file_name = "random." + extension;
			}
			std::string	path = "upload/" + (*it).second.file_name;
			int	toStore;
			if ((*it).second.data.length() - (*it).second.pos > 1024)
				toStore = 1024;
			else
				toStore = (*it).second.data.length() - (*it).second.pos;
			if (!this->writeMultipart.is_open())
				this->writeMultipart.open(path, std::ios::binary | std::ios_base::app);
			if (!this->writeMultipart.good())
				throw std::runtime_error(E500);
			std::string	store = (*it).second.data.substr((*it).second.pos, toStore);
			// this->writeMultipart.write(store.c_str(), store.size());
			this->writeMultipart << store;
			this->writeMultipart.close();
			(*it).second.pos += toStore;
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
				"Content-Length:" + ss.str() + " \r\n"
				"Connection: close\r\n\r\n" + this->body;
	}
}

void	Response::handleNormalBody()
{
	if (this->responseSent == 1 && this->readPos == this->client->request->bodyString.length())
	{
		this->client->request->state = DONE;
		this->fileWrite.close();
		return ;
	}
	if (this->readPos < this->client->request->bodyString.length())
	{
		if (this->started == 0)
		{
			std::map<std::string, std::string>::iterator	it;
			std::string	extension;
			for (it = this->contentTypes.begin(); it != this->contentTypes.end(); it++)
			{
				if ((*it).second == (this->client->request->contentType))
				{
					extension = (*it).first;
					break ;
				}
			}
			if (it == this->contentTypes.end())
				extension = "txt";
			static std::string	path = "upload/random." + extension;
			this->started = 1;
			this->fileWrite.open(path, std::ios_base::app);
			if (!this->fileWrite.good())
				throw std::runtime_error(E500);
		}
		int	toStore;
		if (this->client->request->bodyString.length() - this->readPos > 1024)
			toStore = 1024;
		else
			toStore = this->client->request->bodyString.length() - this->readPos;
		std::string store = this->client->request->bodyString.substr(this->readPos, toStore);
		this->fileWrite << store;
		this->readPos += toStore;
	}
	if (this->responseSent == 0)
	{
		this->body = "<html><head></head><body><h1>KOULCHI NADI AWLDI</h1></body></html>";
		std::stringstream ss;
		ss << this->body.length();
		this->responseStr = "HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length:" + ss.str() + " \r\n"
				"Connection: close\r\n\r\n" + this->body;
	}
}