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
		int									CONTENT_LENGTH;
		std::string							REQUEST_METHOD;
		std::string							PATH_INFO;
		std::string							SCRIPT_FILENAME;
		std::string							CONTENT_TYPE;
		std::string							BODY;
		std::string							QUERY_STRING;
		std::string							CGI_PATH;
		std::vector<std::string>			FULLBODY;
		std::map<std::string, std::string>	QUERY_MAP;
		std::map<std::string, std::string> 	CompilerPathsByLanguage;
		// std::vector<int> 					pids;

		//*********        functions        *********//
		std::string							parseUrl(std::string url);
		std::string							parseSurfix(std::string path_info);
		std::map<std::string, std::string>	parseQuery(std::string query);
};