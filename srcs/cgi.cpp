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
	// this->FULLBODY = make;
	this->QUERY_MAP = std::map<std::string, std::string>();

	this->CompilerPathsByLanguage = std::map<std::string, std::string>{
		{"py", "/usr/bin/python3"},
		{"php", "/usr/bin/php"}
	};    

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


/*     			TO DO				*/
// 0. check the macthing location
// 1. check if the file is executable
// 2. check if the file is a cgi file
// 3. check if the file is a python file
// 4. check if the file is a php file
// 5. check if the file is a bash file
// 6. check if the file is a c file
// 7. check if the file is a c++ file
// 8. check if method is allowed
// 9. check if the file is a directory
// 10. check if the file is a file
// 11. check which compiler to use

void Client::cgi_handler(){

    std::vector<Location>   candidates;

	if (this->request->method == "GET" || (this->request->method == "POST" && (unsigned long)this->request->contentLength == this->request->bodyString.size())){
		std::cout << "cgi_handler" << std::endl;
        candidates = this->response->getLocations(this->server->locations);
		/****************************************************************/
		int		pipefd[2];
		pid_t	pid = -1;
		std::vector<std::string> 							allowed_meth;
        std::vector<Location>::iterator                     iter_cand;
		std::vector<std::string>::iterator					iter_meth;
		std::map<std::string, std::string>::iterator		iter_query;
		std::string 										compiler;
		std::map<std::string, std::string>	querys_map		= this->cgi->parseQuery(this->request->url);
		std::string query_string							= this->request->url.find_first_of("?") != std::string::npos ? this->request->url.substr(this->request->url.find_first_of("?") + 1) : "";
		std::string file_path								= this->cgi->parseUrl(this->request->url);
		std::string server_path								= "/Users/rsaf/Desktop/serveme/cgi-bin" + file_path;
		std::string surfix									= this->cgi->parseSurfix(file_path);
		// std::string compiler								= this->cgi->CompilerPathsByLanguage[surfix];
		//--------------------------------------------------------------
        surfix = "\\." + surfix + "$";
		std::cout << "surfix: " << surfix << std::endl;

		try {
        	for (iter_cand = candidates.begin(); iter_cand < candidates.end(); iter_cand++)
        	{
        	    if (!strcmp(surfix.c_str(), iter_cand->path.c_str()))
				{
					allowed_meth = iter_cand->allowed_methods;
					for (std::map<std::string, std::vector<std::string>>::iterator iter_compiler = iter_cand->location_directives.begin(); iter_compiler != iter_cand->location_directives.end(); iter_compiler++){
						if (iter_compiler->first == "fastcgi_pass"){
							compiler = iter_compiler->second[0];
						}
					}
        	        break;
        		}
			}
			std::cout << "compiler: " << compiler << std::endl;
			std::cout << "file_path: " << file_path << std::endl;
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
				throw this->response->generateError(E504, 0);
			}
			/*	**************************************	*/

			if (pipe(pipefd) == -1)
				throw this->response->generateError(E503, 0);
			if ((pid = fork()) == -1)
				throw this->response->generateError(E503, 0);
			/*child process*/
			std::string tmp_filename =  std::string("tmp/serveme-") + std::to_string(rand()) + ".tmp";
			if (pid == 0) {
				try{
					if (this->request->method == "POST"){
						srand(time(NULL));
						std::ofstream ofs(tmp_filename);
						if (!ofs.is_open())
							throw this->response->generateError(E503, 0);
						ofs << this->request->bodyString;
						ofs.close();
						int fdf = open(tmp_filename.c_str(), O_RDWR);
						if (fdf == -1)
							throw this->response->generateError(E503, 0);
						if (dup2(fdf, STDIN_FILENO) == -1)
							throw this->response->generateError(E503, 0);
						close(fdf);
					}

					/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
					setenv("REQUEST_METHOD", this->request->method.c_str(), 1);
					setenv("REQUEST_URI", this->request->url.c_str(), 1);
					setenv("CONTENT_LENGTH", std::to_string(this->request->contentLength).c_str(), 1);
					setenv("SCRIPT_FILENAME", server_path.c_str(), 1);
					setenv("SCRIPT_NAME", file_path.c_str(), 1);
					setenv("CONTENT_TYPE", this->request->contentType.c_str(), 1); // empty [FIXED]
					setenv("CONTENT_BODY", this->request->bodyString.c_str(), 1);
					setenv("QUERY_STRING", query_string.c_str(), 1);
					setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
					setenv("PATH_INFO", this->request->url.c_str(), 1);
					setenv("REDIRECT_STATUS", "1", 1); // for later
					for (iter_query = this->cgi->QUERY_MAP.begin(); iter_query != this->cgi->QUERY_MAP.end(); ++iter_query)
					     setenv(iter_query->first.c_str(), iter_query->second.c_str(), 1);
					/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
					if (dup2(pipefd[1], STDOUT_FILENO) == -1)
						throw this->response->generateError(E503, 0);
					close(pipefd[0]);
					close(pipefd[1]);

        	    	extern	char**	environ;
        	    	char**	env	= environ;
					file_path.erase(0, 1);
					if (access(file_path.c_str(), F_OK) == -1)
						throw this->response->generateError(E503, 0);
        	    	char*	arg[] = {strdup(compiler.c_str()), strdup(file_path.c_str()), NULL};
        	    	char*	path = strdup(compiler.c_str());
					unlink(tmp_filename.c_str());
					if (execve(path, arg, env) == -1)
						throw this->response->generateError(E503, 0);
				} catch (std::string body){
					std::cout << body;
					exit(1);
				}
			} if (pid != 0) {
				char buff;
				std::string body;
				int error_status;
				wait(&error_status);
				if (error_status != 0) {
					close(pipefd[0]);
					close(pipefd[1]);
					this->request->url = "";
					this->request->bodyString = "";
					unlink(tmp_filename.c_str());
					throw this->response->generateError(E503, 0);
				}
				close(pipefd[1]);
				while (read(pipefd[0], &buff, 1) > 0){
					body.push_back(buff);
				}
        	    std::string header = "HTTP/1.1 200 OK\r\n";
        	    body = body.substr(body.find("\r\n\r\n") + 4);
        	    header += "Content-Type: text/html\r\n";
        	    header += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        	    header += "Server: serveme/1.0\r\n";
        	    header += "Connection: close\r\n\r\n";
        	    header += body;
				this->request->bodyString = "";
        	    int bytes = send(this->request->client_fd, header.c_str(), header.size(), 0);
				if (bytes == -1)
					throw this->response->generateError(E503, 0);
				this->request->state = DONE;
			}
			unlink(tmp_filename.c_str());
			close(pipefd[0]);
		}
		catch (std::string body){
			int bytes = send(this->request->client_fd, body.c_str(), body.size(), 0);
			if (bytes == -1)
				return;
		}
	}
}
