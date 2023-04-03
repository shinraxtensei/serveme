# include "../inc/cgi.hpp"
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <stdlib.h>
#include <string>
#include <sys/_types/_pid_t.h>
#include <sys/fcntl.h>
#include <unistd.h>


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
// int main()
// {
//     std::string url;
//     Cgi cgi;
//     url = cgi.parseUrl("/with/additional/path?query=string");
//     std::cout << url << std::endl;
//     return 0;
// }


void Client::cgi_handler(){


	if (this->request->method == "GET" || (this->request->method == "POST" && (unsigned long)this->request->contentLength == this->request->bodyString.size())){

		/*     TO DO	*/
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


		/****************************************************************/
		std::map<std::string, std::string>::iterator		it;
		std::map<std::string, std::string>	querys_map		= this->cgi->parseQuery(this->request->url);
		std::string query_string							= this->request->url.find_first_of("?") != std::string::npos ? this->request->url.substr(this->request->url.find_first_of("?") + 1) : "";
		std::string file_path								= this->cgi->parseUrl(this->request->url);
		std::string server_path								= "/Users/rsaf/Desktop/serveme/cgi-bin" + file_path;
		std::string surfix									= this->cgi->parseSurfix(file_path);
		std::string compiler								= this->cgi->CompilerPathsByLanguage[surfix];
		//--------------------------------------------------------------
		int		pipefd[2];
		pid_t	pid = -1;


		if (pipe(pipefd) == -1)
			throw std::runtime_error("CGI : Pipe failed");
		if ((pid = fork()) == -1)
			throw std::runtime_error("CGI : Fork failed");
		/*child process*/
		if (pid == 0) {
			if (this->request->method == "POST"){
				std::cout << "[ contentLength = " << this->request->contentLength << " ] [ body size " << this->request->bodyString.size() << "]" << std::endl;
				srand(time(NULL));
				std::string tmp_filename =  std::string("tmp/serveme-") + std::to_string(rand()) + ".tmp";
				std::ofstream ofs(tmp_filename);
				if (!ofs.is_open())
					throw std::runtime_error("CGI : Can't open tmp file");
				ofs << this->request->bodyString;
				std::cout << "body = " << this->request->bodyString << std::endl;
				ofs.close();
				int fdf = open(tmp_filename.c_str(), O_RDWR);
				if (fdf == -1)
					throw std::runtime_error("CGI : Can't open tmp file");
				std::cout << "fdf = " << fdf << std::endl;
				dup2(fdf, STDIN_FILENO);
				close(fdf);
			}
			/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
			setenv("REQUEST_METHOD", this->request->method.c_str(), 1);
			setenv("REQUEST_URI", this->request->url.c_str(), 1);
			setenv("CONTENT_LENGTH", std::to_string(this->request->contentLength).c_str(), 1);
			setenv("SCRIPT_FILENAME", server_path.c_str(), 1);
			setenv("SCRIPT_NAME", file_path.c_str(), 1);
			setenv("CONTENT_TYPE", "application/json", 1); // empty
			std::cout << "CONTENT_TYPE = " << this->request->contentType << std::endl;
			setenv("CONTENT_BODY", this->request->bodyString.c_str(), 1);
			setenv("QUERY_STRING", query_string.c_str(), 1);
			setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);

			if (getenv("REQUEST_METHOD") == NULL)
				throw std::runtime_error("CGI : Can't set env");
			else
			 	std::cout << "REQUEST_METHOD = " << getenv("REQUEST_METHOD") << std::endl;
			// std::cout << "Path info: " << this->cgi->parseUrl(this->request->url) << std::endl;
			setenv("PATH_INFO", this->request->url.c_str(), 1);
			setenv("REDIRECT_STATUS", "1", 1); // for later
			for (it = this->cgi->QUERY_MAP.begin(); it != this->cgi->QUERY_MAP.end(); ++it)
			     setenv(it->first.c_str(), it->second.c_str(), 1);
			/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
			if (dup2(pipefd[1], STDOUT_FILENO) == -1)
				throw std::runtime_error("CGI : Dup2 failed");
			close(pipefd[0]);
			close(pipefd[1]);

            extern	char**	environ;
            char**	env	= environ;
			file_path.erase(0, 1);
            char*	arg[] = {strdup(compiler.c_str()), strdup(file_path.c_str()), NULL};
            char*	path = strdup(compiler.c_str());
			std::cerr << "path: " << path << std::endl;
			std::cerr << "arg[0]: " << arg[0] << std::endl;
			std::cerr << "arg[1]: " << arg[1] << std::endl;
			if (execve(path, arg, env) == -1)
				throw std::runtime_error("CGI : Execve failed");
			exit(1);
		}
    	char buff;
    	std::string body;
    	waitpid(-1, 0, 0);
    	close(pipefd[1]);
    	while (read(pipefd[0], &buff, 1) > 0){
    	    body.push_back(buff);
    	}
    	close(int(pipefd[0]));
	    int bytes = send(this->request->client_fd, body.c_str(),  body.size(), 0);
	    if (bytes == -1)
	        std::cout << "Return 503 ERROR" << std::endl;
	}
}

// void Client::cgi_handler(){

//     std::cout << "******************** cgi_handler **********************" << std::endl;

//     // there's a problem in the allowed methods from request
//     std::vector<std::string> allowed_methods = {"GET", "POST",  "DELETE"};
//     std::vector<std::string>::iterator it;

//     this->cgi->CONTENT_LENGTH   = this->request->contentLength;
//     this->cgi->PATH_INFO        = this->cgi->parseUrl(this->request->url);
//     this->cgi->QUERY_MAP        = this->cgi->parseQuery(this->request->url);
//     this->cgi->BODY             = this->request->bodyString;
//     this->cgi->SCRIPT_FILENAME  = this->request->url;
//     this->cgi->CONTENT_TYPE     = this->request->contentType;
//     std::string surfix          = this->cgi->parseSurfix(this->cgi->PATH_INFO);
//     this->cgi->CGI_PATH         = this->cgi->CompilerPathsByLanguage[surfix];

//     if (this->cgi->CGI_PATH == ""){
//         std::cout << "Return 404 ERROR" << std::endl;
//         exit(1);
//     }
//     it = find (allowed_methods.begin(), allowed_methods.end(), this->request->method);
//     if (it == allowed_methods.end()){
//         std::cout << "Return 405 ERROR" << std::endl;
//         exit(100);
//     }
//     int piepfd[2];
//     if (pipe(piepfd) == -1)
//         std::cout << "Return 503 ERROR" << std::endl;
	
//     /* *************************************************************************************************************** */
//     if (this->request->method == "GET")
//     {
//         // here just teating the file output

//         std::cout << "********* CGI FOR GET IS CALLEEDDDD **********" << std::endl;
//         pid_t pid = fork();
//         if (pid == -1)
//             std::cout << "Return 503 ERROR" << std::endl;
//         /* child process */
//         if (pid == 0){
//             dup2(int(piepfd[1]), 1);
//             close(int(piepfd[0]));
//             close(int(piepfd[1]));
//             try {
//                 this->cgi->setEnv(this->request->method);
//                 extern char** environ;
//                 char** env = environ;
                // this->cgi->PATH_INFO.erase(0, 1);
//                 char* arg[] = {strdup(this->cgi->CGI_PATH.c_str()), strdup(this->cgi->PATH_INFO.c_str()), NULL};
//                 char* path = strdup(this->cgi->CGI_PATH.c_str());
				
//                 if (execve(path, arg, env) == -1)
//                     std::cout << "Return 503 ERROR" << std::endl;
//                 exit(0);
//             } catch (...) {
//                 std::cout << "Error" << std::endl;
//             }
//             exit (1);
//         }
//     }
//     /* *************************************************************************************************************** */
//     else if (this->request->method == "POST")
//     {

//         std::cout << "********* CGI FOR POST IS CALLEEDDDD **********" << std::endl;
//         std::cout << "METHOD : " << this->request->method << std::endl;
//         std::cout << "CONTENT LENGTH : " << this->cgi->CONTENT_LENGTH << std::endl;
//         std::cout << "PATH INFO : " << this->cgi->PATH_INFO << std::endl;
//         std::cout << "BODY : " << this->cgi->BODY << std::endl;
//         std::cout << "SCRIPT FILE NAME : " << this->cgi->SCRIPT_FILENAME << std::endl;
//         std::cout << "CONTENT TYPE : " << this->request->contentType << std::endl;
//         std::cout << "SURFIX : " << surfix << std::endl;
//         std::cout << "CGI PATH : " << this->cgi->CGI_PATH << std::endl;
//         std::cout << "CGI PATH : " << this->cgi->CompilerPathsByLanguage[surfix] << std::endl;
//         pid_t pid = fork();
//         if (pid == -1)
//             std::cout << "Return 503 ERROR" << std::endl;
//         /* child process */
//         if (pid == 0){
//             std::cout << "-------CHILD PROCESS BEGIN POST-------" << std::endl;
		
		
//             std::string chnck = this->request->ParseBody();
//             std::size_t found = chnck.find("\r\n\r\n");
//             std::string bb = chnck.substr(found + 4);
//             std::cout << "Body From Parser Direct :\n" << bb << std::endl;
//             std::ofstream tmp("tmp.txt");
//             tmp << bb;
//             tmp.close();
//             int fd = open("tmp.txt", O_RDONLY | O_CREAT | O_TRUNC, 0666);
//             dup2(fd, 0);
//             close(fd);
//             // dup2(piepfd[1], 1);
//             close(piepfd[0]);
//             close(piepfd[1]);
//             try {
//                 this->cgi->setEnv(this->request->method);
//                 extern char** environ;
//                 char** env = environ;
//                 this->cgi->PATH_INFO.erase(0, 1);
//                 char* arg[] = {strdup(this->cgi->CGI_PATH.c_str()), strdup(this->cgi->PATH_INFO.c_str()), NULL};
//                 char* path = strdup(this->cgi->CGI_PATH.c_str());
//                 if (execve(path, arg, env) == -1)
//                     std::cout << "Return 503 ERROR" << std::endl;
//             } catch (...) {
//                 std::cout << "Error" << std::endl;
//             }
//             std::cout << "-------CHILD PROCESS END POST-------" << std::endl;
//             exit (1);
//         }
//         unlink("tmp.txt");
//     }
//     std::cout << "Parent Process" << std::endl;
//     char buff;
//     std::string body;
//     waitpid(-1, 0, 0);
//     std::cerr << "heeeere" << std::endl;
//     // int flags = fcntl(piepfd[0], F_GETFL, 0);
//     // fcntl(piepfd[0], F_SETFL, flags | O_NONBLOCK);
//     close(piepfd[1]);
//     while (read(piepfd[0], &buff, 1) > 0){
//         // std::cout << "BODY: " << buff << std::endl;
//         body.append(buff, 1);
//     }
//     close(int(piepfd[0]));
//     std::cout << "-----------------BODY FROM CGI-----------------\n" << body << std::endl;
//     std::cerr  << "|" << body << "|" << std::endl;
//     // int bytes = send(this->request->client_fd, body.c_str(),  body.size(), 0);
//     // if (bytes == -1)
//     //     std::cout << "Return 503 ERROR" << std::endl;
// }