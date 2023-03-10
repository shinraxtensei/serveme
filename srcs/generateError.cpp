#include "../inc/macros.hpp"
#include "../inc/servme.hpp"

std::string	generateError(std::string error)
{
	std::string del = " ";
	std::string code;
	code = error.substr(0, error.find(del));

	if (*(code.begin()) != '2')
	{
		std::ofstream	errorFile;
		errorFile.open("/Users/yabtaour/Desktop/webserv-42/errors/" + code + ".html");
		if (!errorFile.is_open())
			throw std::runtime_error("Error opening file");
		errorFile << "<!DOCTYPE html>\n";
		errorFile << "<html>\n";
		errorFile << "<head>\n";
		errorFile << "<title>Serverme</title>\n";
		errorFile << "</head>\n";
		errorFile << "<body>\n";
		errorFile << "<h1>" << error << "</h1>\n";
		errorFile << "</body>\n";
		errorFile << "</html>\n";
		errorFile.close();
	}
	return (code);
}