#pragma once

#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <signal.h>
#include <string.h>
#include "parser.hpp"
#include "socketWrapper.hpp"
#include "server.hpp"



// *socket wrapper :



class Http;
// class Core
// {
// public:
//     // TODO : create a wrapper for all sockets functions
//     static std::set<SocketWrapper> serverSockets;
//     static std::vector<Client> clients;
//     static void handleConnections();
//     static void HandleResquest(int fd);
//     static Http *get_http();
//     // static void soket();
//     static void startup();
// };


class Core
{
    public:
		std::map<std::string, std::string> mimeTypes;
    	std::vector<SocketWrapper> serverSockets;
    	std::vector<Client> clients;

    	Core(){};
    	~Core()
    	{
        	for (size_t i = 0; i < this->serverSockets.size(); i++)
        		this->serverSockets[i].~SocketWrapper();
        	for (size_t i = 0; i < this->clients.size(); i++)
            	this->clients[i].~Client();
    	}

		void	parseMimeTypes();
    	int check_servers_socket(int fd);
    	void handleConnections();
    	void HandleResquest(pollfd FD);
    	Http *get_http();
    	void startup();
    	void checkInactivity();
};

