#include "../inc/client.hpp"
#include "../inc/macros.hpp"

void	Response::handleDelete()
{
	struct    stat    infos;

	if (this->responseSent == 0)
	{
		std::string	body = "<html><head></head><body><h1>FILE DELETED :))</h1></body>";
    	if (stat(newPath.c_str(), &infos) == 0)
		{
			if (remove(newPath.c_str()) == 0)
			{
				this->responseStr = "HTTP/1.1 200 OK\r\n"
									"Content-Type: text/html\r\n"
									"Content-Length: " + std::to_string(body.length()) + "\r\n"
									"Connection: keep-alive\r\n\r\n"
									 + body;
				this->client->request->state = DONE;
			}
			else
				throw std::runtime_error(E403);
		}
		else
			throw std::runtime_error(E404);
	}
}