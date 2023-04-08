#include "../inc/macros.hpp"
#include "../inc/servme.hpp"

std::string	Response::generateError(std::string error, int flag)
{
	std::string	body;	
	if (flag == DEFAULT)
		body = this->body;
	else
		body = "<!DOCTYPE html>\n<html>\n<head>\n<title>"
						+ error
						+ "</title>\n</head>\n<body>\n<h1>"
						+ error
						+ "</h1>\n</body>\n</html>";
	size_t	size = body.size();
	std::stringstream ss;
	ss << size;
	std::string	headers = "HTTP/1.1 " + error
						+ "\r\nContent-Length: "
						+ ss.str()
						+ "\r\nContent-Type: text/html"
						+ "\r\nServer: a7san/server/2.0"
						+ "\r\n\r\n";
	return (headers + body);
}