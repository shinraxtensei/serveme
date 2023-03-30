#include "../inc/macros.hpp"
#include "../inc/servme.hpp"

std::string	generateError(std::string error)
{
	std::string	body = 	"<!DOCTYPE html>\n<html>\n<head>\n<title>"
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
	// errorPair.first = "HTTP/1.1 " + error;
	// errorPair.second = "Content-Length: "
	// 					"Content-Type: text/html"
	// 					"Server: a7san/server/2.0"
	// 					+ "\r\n"
	// 					"Content-Type: text/html\r\n\r\n"
	// 					+ body;
	return (headers + body);
}