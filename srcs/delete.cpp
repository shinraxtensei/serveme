#include "../inc/client.hpp"
#include "../inc/macros.hpp"

void	Response::handleDelete(std::string newPath)
{
	struct    stat    infos;

	std::cout << " trying to delete new Path : " << newPath << std::endl;
    if (stat(newPath.c_str(), &infos) == 0)
	{
        std::cout << "File found " << this->client->location->path.c_str() << std::endl;
		if (remove(newPath.c_str()) == 0)
		{
			this->responseStr = "HTTP/1.1 200 OK\r\n"
								"Content-Type: text/html\r\n"
								"Content-Length: 0\r\n"
								"Connection: close\r\n\r\n";
			send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		}
		else
		{
			if (checkError(403))
				this->responseStr = generateError(E403, DEFAULT);
			else
				this->responseStr = generateError(E403, MINE);
			send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
			// exit (1);
		}
	}
	else
	{
		if (checkError(404))
			this->responseStr = generateError(E404, DEFAULT);
		else
			this->responseStr = generateError(E404, MINE);
		send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		// exit (1);
	}
}