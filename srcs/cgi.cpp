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
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>


Cgi::Cgi(){
	this->QUERY_MAP = std::map<std::string, std::string>();
	this->state = 0;
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

void Client::checkReturn(){
		std::string body = "";
		if (this->server->returned != 0)
		{
			if (this->server->returnType == "permanent")
				body = "HTTP/1.1 308 Permanent Redirect\r\n"
									"Location: " +
									this->server->returnUrl + "\r\n"
																	  "Content-Type: text/html\r\n"
																	  "Content-Length: 0\r\n"
																	  "Connection: keep-alive\r\n\r\n";
			else
				body = "HTTP/1.1 307 Temporary Redirect\r\n"
									"Location: " +
									this->server->returnUrl + "\r\n"
																	  "Content-Type: text/html\r\n"
																	  "Content-Length: 0\r\n"
																	  "Connection: keep-alive\r\n\r\n";
			int bytes = send(this->request->client_fd, body.c_str(), body.length(), 0);
			if (bytes != 1 || bytes == 0)
				throw this->response->generateError(E500, MINE);
			body = 1;
			this->request->state = DONE;
			return;
		}
}

void Client::checkReturn(std::string url, std::string type){
	std::string body = "";
		if (type == "permanent")
			body = "HTTP/1.1 308 Permanent Redirect\r\n"
								"Location: " +
								url + "\r\n"
																  "Content-Type: text/html\r\n"
																  "Content-Length: 0\r\n"
																  "Connection: keep-alive\r\n\r\n";
		else
			body = "HTTP/1.1 307 Temporary Redirect\r\n"
								"Location: " +
								url + "\r\n"
																  "Content-Type: text/html\r\n"
																  "Content-Length: 0\r\n"
																  "Connection: keep-alive\r\n\r\n";
		int bytes = send(this->request->client_fd, body.c_str(), body.length(), 0);
		if (bytes != 1 || bytes == 0)
			throw 500;
		body = 1;
		this->request->state = DONE;
		return;
}

void Client::cgi_handler(){
    std::vector<Location>   candidates;

	if (this->server->returned == 1){
		this->checkReturn();
		return ;
	}
	if (!this->cgi->state && (this->request->method == "GET" || (this->request->method == "POST" && (unsigned long)this->request->contentLength == this->request->bodyString.size()))){
        candidates = this->response->getLocations2(this->server->locations);
		/****************************************************************/
		int		pipefd[2];
		srand(time(NULL));
		struct stat infos;
		std::vector<std::string> 							allowed_meth;
        std::vector<Location>::iterator                     iter_cand;
		std::vector<std::string>::iterator					iter_meth;
		std::map<std::string, std::string>::iterator		iter_query;
		std::string 										compiler;
		std::string 										tmp_surfix;
		std::map<std::string, std::string>	querys_map		= this->cgi->parseQuery(this->request->url);
		std::string query_string;
		if (this->request->url.find_first_of("?") != std::string::npos)
			query_string = this->request->url.substr(this->request->url.find_first_of("?") + 1);
		else
			query_string = "";
		std::string file_path								= this->cgi->parseUrl(this->request->url);
		std::string server_path								= SERVER_PATH + file_path;
		std::string state_path								= file_path.substr(1);
		std::string surfix		 = this->cgi->parseSurfix(file_path);
		std::string tmp_filename =  std::string("tmp/serveme-") + std::to_string(rand()) + ".tmp";
		std::string cookie_value = this->response->parseCookies();
		cookie_value = cookie_value.substr(0, cookie_value.find("\n") - 1);
        tmp_surfix = "\\." + surfix + "$";
		try {
			if (stat(state_path.c_str(), &infos) == -1)
				throw this->response->generateError(E404, 0);
			else if (S_ISDIR(infos.st_mode)){
				this->response->handleNormalReq();
				return;
			}
        	for (iter_cand = candidates.begin(); iter_cand < candidates.end(); iter_cand++)
        	{
        	    if (!strcmp(tmp_surfix.c_str(), iter_cand->path.c_str()))
				{
					allowed_meth = iter_cand->allowed_methods;
					for (std::map<std::string, std::vector<std::string> >::iterator iter_compiler = iter_cand->location_directives.begin(); iter_compiler != iter_cand->location_directives.end(); iter_compiler++){
						if (iter_compiler->first == "return" && iter_cand->returned == 1){
							this->checkReturn(iter_cand->returnUrl, iter_cand->returnType);
							return;
						}
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
			if ((this->pid  = fork()) == -1)
				throw this->response->generateError(E503, 0);
			/*child process*/
			if (this->pid  == 0) {
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
					setenv("SCRIPT_NAME", file_path.c_str(), 1);
					setenv("CONTENT_TYPE", this->request->contentType.c_str(), 1); // empty [FIXED]
					setenv("CONTENT_BODY", this->request->bodyString.c_str(), 1);
					setenv("QUERY_STRING", query_string.c_str(), 1);
					setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
					setenv("PATH_INFO", this->request->url.c_str(), 1);
					setenv("REDIRECT_STATUS", "", 1); // for later
					for (iter_query = this->cgi->QUERY_MAP.begin(); iter_query != this->cgi->QUERY_MAP.end(); ++iter_query)
					     setenv(iter_query->first.c_str(), iter_query->second.c_str(), 1);
					if (this->request->method == "POST")
						setenv("CONTENT_LENGTH", std::to_string(this->request->contentLength).c_str(), 1);
					/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
					if (dup2(this->request->client_fd, STDOUT_FILENO) == -1)
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
					std::cout << "HTTP/1.1 200 OK\r\n";
					if (execve(path, arg, env) == -1)
					{
						exit(6);
						throw 500;
					}
				} catch (int error){
					close(pipefd[0]);
					close(pipefd[1]);
					std::string body;
					if (error == 404)
						body = this->response->generateError(E404, 0);
					else if (error == 503)
						body = this->response->generateError(E503, 0);
					else if (error == 500)
						body = this->response->generateError(E500, 0);
					else
						body = this->response->generateError(E503, 0);
					send(this->request->client_fd, body.c_str(), body.size(), 0);
					exit(-2);
				}

			}
			// this->request->state = DONE;
			this->cgi->state = 1;
			unlink(tmp_filename.c_str());
			close(pipefd[0]);
		}
		catch (std::string body){
			this->request->state = DONE;
			int bytes = send(this->request->client_fd, body.c_str(), body.size(), 0);
			if (bytes == -1 || bytes == 0)
				return;
		}
	}
	int status;
	if (this->pid == 0 && this->cgi->state == 0)
		return;
	waitpid(this->pid, &status, WNOHANG);
	if (WIFEXITED(status) || WIFSIGNALED(status))
	{
		this->cgi->state = 0;
		this->request->state = DONE;
	}
}
