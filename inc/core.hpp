#pragma once

#include "parser.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <signal.h>




// *socket wrapper :



class Http;
class Core
{
public:
    // TODO : create a wrapper for all sockets functions


    static void handleConnections();
    static Http *get_http();
    // static void soket();
};