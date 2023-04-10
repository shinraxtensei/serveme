#include "../inc/client.hpp"
#include "../inc/macros.hpp"

void	Response::handleDelete()
{
	struct    stat    infos;

    if (stat(newPath.c_str(), &infos) == 0)
	{
		if (remove(newPath.c_str()) == 0)
		{
			this->responseStr = "HTTP/1.1 200 OK\r\n"
								"Content-Type: text/html\r\n"
								"Content-Length: 0\r\n"
								"Connection: close\r\n\r\n";
			send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
			// dont send here
		}
		else
			throw std::runtime_error(E403);
	}
	else
		throw std::runtime_error(E404);
}