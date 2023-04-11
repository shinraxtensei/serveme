#pragma once

#include "servme.hpp"


class SocketWrapper;
class Client;
class Http;


class pollfd;





class Core
{
    public:
		std::vector<pollfd> pollFds;
		std::map<std::string, std::string> mimeTypes;
		std::map<int, Client> map_clients; //TODO : this should be the new map to store each fd with its client
    	std::vector<Client> clients;
    	std::vector<SocketWrapper> serverSockets;


		

    	Core();
    	~Core();

		void		parseMimeTypes();
		// std::string	checkType(std::string path);    //TODO  : move it to request class
    	int 		check_servers_socket(int fd);
    	void 		handleConnections();
    	Http 		*get_http();
    	void 		startup();
		void 		removeClient(Client &client);
		bool 		check_client_inactivity(Client &client , time_t timeout);
};

