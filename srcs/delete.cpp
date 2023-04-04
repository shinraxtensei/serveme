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
			std::cout << "ma3ndekch l7ee99" << std::endl;
			this->responseStr = generateError(E403);
			send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
			exit (1);
		}
	}
	else
	{
		this->responseStr = generateError(E404);
		send(this->client_fd, this->responseStr.c_str(), this->responseStr.length(), 0);
		exit (1);
	}
}