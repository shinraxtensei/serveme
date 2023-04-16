# include "../inc/cgi.hpp"
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <stdlib.h>
#include <string>
#include <sys/_types/_pid_t.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <vector>


Cgi::Cgi(){
	this->CONTENT_LENGTH = 0;
	this->PATH_INFO = "";
	this->SCRIPT_FILENAME = "";
	this->CONTENT_TYPE = "";
	this->BODY = "";
	this->QUERY_MAP = std::map<std::string, std::string>();

}


// this function parse the url and return the path
std::string Cgi::parseUrl(std::string url){
	std::string parsedUrl = url;
	std::string::size_type pos = parsedUrl.find("?");
	if (pos != std::string::npos)
		parsedUrl = parsedUrl.substr(0, pos);
	return parsedUrl;
}

// this function parse the query and return a map of key value
std::map<std::string, std::string> Cgi::parseQuery(std::string query){

	std::map<std::string, std::string> querymap;

	std::string::size_type pos = query.find("?");
	if (pos != std::string::npos) {
		query = query.substr(pos + 1);
		this->QUERY_STRING = query;
		while (!query.empty()) {
			std::string::size_type pos2 = query.find("&");
			std::string keyvalue = query.substr(0, pos2);
			std::string::size_type pos3 = keyvalue.find("=");
			if (pos3 != std::string::npos) {
				querymap[keyvalue.substr(0, pos3)] = keyvalue.substr(pos3 + 1);
			}
			if (pos2 == std::string::npos) {
				break;
			}
			query = query.substr(pos2 + 1);
		}
	}
	return querymap;
}

std::string Cgi::parseSurfix(std::string path_info){
	std::string surfix;
	std::size_t found = path_info.find_last_of(".");
	surfix = path_info.substr(found + 1);
	return surfix;
}

std::vector<Location>	Response::getLocations2(std::vector<Location> locations)
{
	std::vector<Location>	candidates;
	std::vector<Location>::iterator	iter;
	this->client = Servme::getCore()->map_clients[this->client_fd];

	iter = locations.begin();
	for (iter = this->client->server->locations.begin(); iter < this->client->server->locations.end(); iter++)
	{
			candidates.push_back(*iter);
	}
	return (candidates);
}

void Client::cgi_handler(){
    std::vector<Location>   candidates;

	if (this->request->method == "GET" || (this->request->method == "POST" && (unsigned long)this->request->contentLength == this->request->bodyString.size())){
        candidates = this->response->getLocations2(this->server->locations);
		/****************************************************************/
		int		pipefd[2];
		pid_t	pid = -1;
		srand(time(NULL));
		std::vector<std::string> 							allowed_meth;
        std::vector<Location>::iterator                     iter_cand;
		std::vector<std::string>::iterator					iter_meth;
		std::map<std::string, std::string>::iterator		iter_query;
		std::string 										compiler;
		std::string 										tmp_surfix;
		std::map<std::string, std::string>	querys_map		= this->cgi->parseQuery(this->request->url);
		std::string query_string							= this->request->url.find_first_of("?") != std::string::npos ? this->request->url.substr(this->request->url.find_first_of("?") + 1) : "";
		std::string file_path								= this->cgi->parseUrl(this->request->url);
		std::string server_path								= "/Users/rsaf/Desktop/serveme" + file_path;
		std::string surfix									= this->cgi->parseSurfix(file_path);
		std::string tmp_filename =  std::string("tmp/serveme-") + std::to_string(rand()) + ".tmp";
		std::string cookie_value = this->response->parseCookies();
		cookie_value = cookie_value.substr(0, cookie_value.find("\n") - 1);
		//--------------------------------------------------------------
        // surfix = "\\." + surfix + "$";
        tmp_surfix = "\\." + surfix + "$";
		try {
        	for (iter_cand = candidates.begin(); iter_cand < candidates.end(); iter_cand++)
        	{
        	    if (!strcmp(tmp_surfix.c_str(), iter_cand->path.c_str()))
				{
					allowed_meth = iter_cand->allowed_methods;
					for (std::map<std::string, std::vector<std::string> >::iterator iter_compiler = iter_cand->location_directives.begin(); iter_compiler != iter_cand->location_directives.end(); iter_compiler++){
						if (iter_compiler->first == "fastcgi_pass"){
							compiler = iter_compiler->second[0];
						}
					}
        	        break;
        		}
			}
			if (compiler == ""){
				std::string body = this->response->generateError(E503, 0);
				throw body;
			}

			if (iter_cand == candidates.end())
				throw this->response->generateError(E503, 0);
			/*	**************************************	*/
			for (iter_meth = allowed_meth.begin(); iter_meth != allowed_meth.end(); iter_meth++){
				if (this->request->method == *iter_meth)
					break;
			}
			if (iter_meth == allowed_meth.end()){
				throw this->response->generateError(E405, 0);
			}
			/*	**************************************	*/
			if (pipe(pipefd) == -1)
				throw this->response->generateError(E503, 0);
			if ((pid = fork()) == -1)
				throw this->response->generateError(E503, 0);
			/*child process*/
			if (pid == 0) {
				try{
					if (this->request->method == "POST"){
						std::ofstream ofs(tmp_filename);
						if (!ofs.is_open())
							throw 503;
						ofs << this->request->bodyString;
						ofs.close();
						int fdf = open(tmp_filename.c_str(), O_RDWR);
						if (fdf == -1)
							throw 503;
						if (dup2(fdf, STDIN_FILENO) == -1)
							throw 503;
						close(fdf);
					}
					/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
					if (cookie_value.size() > 0)
						setenv("HTTP_COOKIE", cookie_value.c_str(), 1);
					setenv("REQUEST_METHOD", this->request->method.c_str(), 1);
					setenv("REQUEST_URI", this->request->url.c_str(), 1);
					setenv("SCRIPT_FILENAME", server_path.c_str(), 1);
					setenv("SCRIPT_FILENAME", "/Users/rsaf/Desktop/serveme/cgi-bin/user.php" , 1);
					setenv("SCRIPT_NAME", file_path.c_str(), 1);
					setenv("CONTENT_TYPE", this->request->contentType.c_str(), 1); // empty [FIXED]
					setenv("CONTENT_BODY", this->request->bodyString.c_str(), 1);
					setenv("QUERY_STRING", query_string.c_str(), 1);
					setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
					setenv("PATH_INFO", "/Users/rsaf/Desktop/serveme/", 1);
					setenv("PATH_INFO", this->request->url.c_str(), 1);
					setenv("REDIRECT_STATUS", "1", 1); // for later
					for (iter_query = this->cgi->QUERY_MAP.begin(); iter_query != this->cgi->QUERY_MAP.end(); ++iter_query)
					     setenv(iter_query->first.c_str(), iter_query->second.c_str(), 1);
					if (this->request->method == "POST")
						setenv("CONTENT_LENGTH", std::to_string(this->request->contentLength).c_str(), 1);
					/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
					if (dup2(pipefd[1], STDOUT_FILENO) == -1)
						throw 503;
					close(pipefd[0]);
					close(pipefd[1]);
        	    	extern	char**	environ;
        	    	char**	env	= environ;
					file_path.erase(0, 1);
					if (access(file_path.c_str(), F_OK) == -1)
						throw 404;
        	    	const char*	path = compiler.c_str();
        	    	char*	arg[] = {(char *)path, (char *)file_path.c_str(), NULL};
					unlink(tmp_filename.c_str());
					if (execve(path, arg, env) == -1)
						throw 500;
				} catch (int error){
					close(pipefd[0]);
					close(pipefd[1]);
					if (error == 404)
						exit(4);
					else if (error == 503)
						exit(5);
					else if (error == 500)
						exit(6);
					else
						exit(-1);
				}
			} 
			int error_status;
			wait(&error_status);
			char buff;
			std::string body;
			if (error_status != 0) {
				close(pipefd[0]);
				close(pipefd[1]);
				unlink(tmp_filename.c_str());
				if (WEXITSTATUS(error_status) == 4)
					throw this->response->generateError(E404, 0);
				else if (WEXITSTATUS(error_status) == 5)
					throw this->response->generateError(E503, 0);
				else if (WEXITSTATUS(error_status) == 6)
					throw this->response->generateError(E500, 0);
				else
					throw this->response->generateError(E503, 0);
			}
			close(pipefd[1]);
			while (read(pipefd[0], &buff, 1) > 0){
				body.push_back(buff);
			}
        	std::string header;
        	body = body.substr(body.find("\r\n\r\n") + 4);
			cookie_value = cookie_value.substr(cookie_value.find("=") + 1, cookie_value.find(";"));
			if (cookie_value != "" || querys_map.find("color") != querys_map.end()){
				if (cookie_value != ""){
					header = "HTTP/1.1 200 OK\r\n";
					header += "Set-Cookie: color=" + cookie_value + "\r\n";
				}
				if (querys_map.find("color") != querys_map.end()){
					header = "HTTP/1.1 200 OK\r\n";
					header += "Set-Cookie: color=" + querys_map["color"] + "\r\n";
				}
			}
			else
				header = "HTTP/1.1 200 OK\r\n";
        	header += "Content-Type: text/html\r\n";
        	header += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        	header += "Server: server/" + this->server->server_name + "--v Beta" + "\r\n";
        	header += "Connection: keep-alive\r\n\r\n";
        	header += body;
        	int bytes = send(this->request->client_fd, header.c_str(), header.size(), 0);
			if (bytes == -1)
				throw this->response->generateError(E503, 0);
			header = "";
			this->request->state = DONE;
			unlink(tmp_filename.c_str());
			close(pipefd[0]);
		}
		catch (std::string body){
			this->request->state = DONE;
			int bytes = send(this->request->client_fd, body.c_str(), body.size(), 0);
			if (bytes == -1)
				return;
		}
	}
}
