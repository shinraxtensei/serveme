#include "../inc/client.hpp"

void	Response::checkAllowedMethods()
{
	std::vector<std::string>			methods;
	std::vector<std::string>::iterator	iter;

	methods = this->client->server->allowed_methods;
	if (methods.size() == 0 && this->client->request->method == "GET")
		return ;
	for (iter = methods.begin(); iter < methods.end(); iter++)
	{
		if (*iter == this->client->request->method)
			break ;
	}
	if (iter == methods.end())
		std::cout << "method not allowed" << std::endl;
}

void	Response::matchLocation()
{
	std::vector<std::string>::iterator	iter;
	std::vector<std::string>			imaginaryLocations;
	std::string							path = "/home/etc/youness";

	imaginaryLocations.push_back("/home/etc");
	imaginaryLocations.push_back("/home/etc/youness/file.txt");
	imaginaryLocations.push_back("/");

	if (path.back() == '/')
		path = path.substr(0, path.size() - 1);

	// std::cout << "trying to match : " << path << std::endl;

	// path = this->client->request->url;
	// locations = this->client->server->locations;
	while (path != "/")
	{
		std::cout << "trying to match : " << path << std::endl;
		for (iter = imaginaryLocations.begin(); iter < imaginaryLocations.end(); iter++)
		{
			if (*iter == path)
				break ;
		}
		if (iter == imaginaryLocations.end())
		{
			path = path.substr(0, path.find_last_of("/"));
			std::cout << path << std::endl;
		}
		else
		{
			std::cout << "location found " << *iter << std::endl;
			break ;
		}
	}
}