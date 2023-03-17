
#pragma once

#include <chrono>

#include <sys/poll.h>
#include <signal.h>
#include <string.h>
#include <string>

#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <set>
#include <unordered_set>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <unordered_map>

#include "cgi.hpp"
#include "client.hpp"
#include "core.hpp"
#include "lexer.hpp"
#include "macros.hpp"
#include "parser.hpp"
#include "socketWrapper.hpp"

std::pair<std::string, std::string> generateError(std::string error);

class Core;

class Servme
{
private:
	static Core *core;

public:
	static Core *getCore();
};
