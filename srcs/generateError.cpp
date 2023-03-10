#include "../inc/macros.hpp"
#include "../inc/servme.hpp"

std::pair<std::string, std::string>	generateError(std::string error)
{
	std::pair<std::string, std::string> errorPair;
	errorPair.first = error;
	errorPair.second = "<!DOCTYPE html>\n<html>\n<head>\n<title>"
						+ error
						+ "</title>\n</head>\n<body>\n<h1>"
						+ error
						+ "</h1>\n</body>\n</html>";
	return (errorPair);
}