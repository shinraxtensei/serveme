#pragma once

#include "servme.hpp"
#include <map>
#include <string>
#include <vector>
class Cgi{

	public:
		//********		constructors 		*********//
		Cgi();
		// Cgi(const Cgi &other);
		// Cgi &operator=(const Cgi &other);
		// ~Cgi();

		//*********        variables        *********//
		int state;
		std::map<std::string, std::string>	QUERY_MAP;
		// std::vector<int> 					pids;

		//*********        functions        *********//
		std::string							parseUrl(std::string url);
		std::string							parseSurfix(std::string path_info);
		std::map<std::string, std::string>	parseQuery(std::string query);
};