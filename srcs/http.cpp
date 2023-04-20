#include "../inc/core.hpp"

Http::Http()
{

    this->index.push_back("");
    this->root = "/html";
    this->autoindex = false;
    this->client_max_body_size = 5242880;
    // this->allowed_methods.push_back("GET");
    // this->allowed_methods.push_back("POST");
    // this->allowed_methods.push_back("DELETE");
}

void Parser::init_http()
{
    if (Parser::getHttp()->http_directives.find("root") != Parser::getHttp()->http_directives.end())
    {
        Parser::getHttp()->root = Parser::getHttp()->http_directives.find("root")->second[0];
        if (Parser::getHttp()->http_directives.find("root")->second.size() != 1)
            throw std::runtime_error("Error: Multiple root directives");
    }

    if (Parser::getHttp()->http_directives.find("index") != Parser::getHttp()->http_directives.end())
        Parser::getHttp()->index = Parser::getHttp()->http_directives.find("index")->second;

    if (Parser::getHttp()->http_directives.find("client_max_body_size") != Parser::getHttp()->http_directives.end())
    {
        Parser::getHttp()->client_max_body_size = atoi(Parser::getHttp()->http_directives.find("client_max_body_size")->second[0].c_str());
        if (Parser::getHttp()->http_directives.find("client_max_body_size")->second.size() != 1)
            throw std::runtime_error("Error: Multiple client_max_body_size directives");
    }

    if (Parser::getHttp()->http_directives.find("error_page") != Parser::getHttp()->http_directives.end())
    {
        std::pair<std::multimap<std::string, std::vector<std::string> >::iterator, std::multimap<std::string, std::vector<std::string> >::iterator> ret;
        ret = Parser::getHttp()->http_directives.equal_range("error_page");
        while (ret.first != ret.second)
        {

            Parser::getHttp()->error_page[atoi(ret.first->second[0].c_str())] = ret.first->second;
            Parser::getHttp()->error_page[atoi(ret.first->second[0].c_str())].erase(Parser::getHttp()->error_page[atoi(ret.first->second[0].c_str())].begin());
            ret.first++;
        }
    }

    if (Parser::getHttp()->http_directives.find("autoindex") != Parser::getHttp()->http_directives.end())
    {
        if (Parser::getHttp()->http_directives.find("autoindex")->second[0] == "on" || Parser::getHttp()->http_directives.find("autoindex")->second.size() != 1)
            Parser::getHttp()->autoindex = true;
        else if (Parser::getHttp()->http_directives.find("autoindex")->second[0] == "off" || Parser::getHttp()->http_directives.find("autoindex")->second.size() != 1)
            Parser::getHttp()->autoindex = false;
        else
            throw std::runtime_error("Error: autoindex can only be on or off");
    }

    if (Parser::getHttp()->http_directives.find("allowed_methods") != Parser::getHttp()->http_directives.end())
        Parser::getHttp()->allowed_methods = Parser::getHttp()->http_directives.find("allowed_methods")->second;
}

Http::~Http()
{
}

Server::Server()
{

    this->ipPort.first = "NONE";
    this->ipPort.second = 6969;

    this->returned = false;
    this->returnType = "";
    this->returnUrl = "";
    this->server_name = "localhost";
}

void Parser::init_servers()
{
    for (size_t i = 0; i < Parser::getHttp()->servers.size(); i++)
    {

        // server_name

        if (Parser::getHttp()->servers[i].server_directives.find("server_name") != Parser::getHttp()->servers[i].server_directives.end())
        {
            Parser::getHttp()->servers[i].server_name = Parser::getHttp()->servers[i].server_directives.find("server_name")->second[0];
            if (Parser::getHttp()->servers[i].server_directives.find("server_name")->second.size() != 1)
                throw std::runtime_error("Error: server_name directive can only have one value");
        }

        // listen
        if (Parser::getHttp()->servers[i].server_directives.find("listen") != Parser::getHttp()->servers[i].server_directives.end())
        {
            std::pair<std::string, std::vector<std::string> > listenDirective = *Parser::getHttp()->servers[i].server_directives.find("listen");
            if (listenDirective.second.size() != 1)
                throw std::runtime_error("Error: listen directive can only have one value");

            else if (listenDirective.second[0].find(":") != std::string::npos)
            {
                Parser::getHttp()->servers[i].ipPort.first = listenDirective.second[0].substr(0, listenDirective.second[0].find(":"));
                Parser::getHttp()->servers[i].ipPort.second = std::stoi(listenDirective.second[0].substr(listenDirective.second[0].find(":") + 1));
            }
            else
            {
                Parser::getHttp()->servers[i].ipPort.first = "NONE";
                Parser::getHttp()->servers[i].ipPort.second = std::stoi(listenDirective.second[0]);
            }
        }

        // root
        if (Parser::getHttp()->servers[i].server_directives.find("root") != Parser::getHttp()->servers[i].server_directives.end())
        {
            if (Parser::getHttp()->servers[i].server_directives.find("root")->second.size() != 1)
            {
                // std::cout  << Parser::getHttp()->servers[i].server_directives.find("root")->second.size() << std::endl;
                throw std::runtime_error("Error: root directive can only have one value");
            }
            Parser::getHttp()->servers[i].root = Parser::getHttp()->servers[i].server_directives.find("root")->second[0];
        }
        else
            Parser::getHttp()->servers[i].root = Parser::getHttp()->root;

        // index
        if (Parser::getHttp()->servers[i].server_directives.find("index") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].index = Parser::getHttp()->servers[i].server_directives.find("index")->second;
        else
            Parser::getHttp()->servers[i].index = Parser::getHttp()->index;

        // error_page

        if (Parser::getHttp()->servers[i].server_directives.find("error_page") != Parser::getHttp()->servers[i].server_directives.end())
        {
            std::pair<std::multimap<std::string, std::vector<std::string> >::iterator, std::multimap<std::string, std::vector<std::string> >::iterator> ret;
            ret = Parser::getHttp()->servers[i].server_directives.equal_range("error_page");
            while (ret.first != ret.second)
            {
                Parser::getHttp()->servers[i].error_page[atoi(ret.first->second[0].c_str())] = ret.first->second;
                Parser::getHttp()->servers[i].error_page[atoi(ret.first->second[0].c_str())].erase(Parser::getHttp()->servers[i].error_page[atoi(ret.first->second[0].c_str())].begin());
                ret.first++;
            }
        }
        else
            Parser::getHttp()->servers[i].error_page = Parser::getHttp()->error_page;

        // autoindex
        if (Parser::getHttp()->servers[i].server_directives.find("autoindex") != Parser::getHttp()->servers[i].server_directives.end())
        {
            // //std::cout << RED << "autoindex: " << Parser::getHttp()->servers[i].server_directives["autoindex"][1] << RESET <<std::endl;
            if (Parser::getHttp()->servers[i].server_directives.find("autoindex")->second[0] == "on" || Parser::getHttp()->servers[i].server_directives.find("autoindex")->second.size() != 1)
                Parser::getHttp()->servers[i].autoindex = true;
            else if (Parser::getHttp()->servers[i].server_directives.find("autoindex")->second[0] == "off" || Parser::getHttp()->servers[i].server_directives.find("autoindex")->second.size() != 1)
                Parser::getHttp()->servers[i].autoindex = false;
        }
        else
            Parser::getHttp()->servers[i].autoindex = Parser::getHttp()->autoindex;

        // client_max_body_size
        if (Parser::getHttp()->servers[i].server_directives.find("client_max_body_size") != Parser::getHttp()->servers[i].server_directives.end())
        {
            Parser::getHttp()->servers[i].client_max_body_size = std::stoi(Parser::getHttp()->servers[i].server_directives.find("client_max_body_size")->second[0]);
            if (Parser::getHttp()->servers[i].server_directives.find("client_max_body_size")->second.size() != 1)
                throw std::runtime_error("Error: client_max_body_size directive can only have one value");
        }
        else
            Parser::getHttp()->servers[i].client_max_body_size = Parser::getHttp()->client_max_body_size;

        // allowed_methods
        if (Parser::getHttp()->servers[i].server_directives.find("allowed_methods") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].allowed_methods = Parser::getHttp()->servers[i].server_directives.find("allowed_methods")->second;
        else
            Parser::getHttp()->servers[i].allowed_methods = Parser::getHttp()->allowed_methods;

        if (Parser::getHttp()->servers[i].server_directives.find("return") != Parser::getHttp()->servers[i].server_directives.end())
        {
            Parser::getHttp()->servers[i].returned = true;

            Parser::getHttp()->servers[i].returnUrl = Parser::getHttp()->servers[i].server_directives.find("return")->second[0];
            Parser::getHttp()->servers[i].returnType = Parser::getHttp()->servers[i].server_directives.find("return")->second[1];
            if (Parser::getHttp()->servers[i].returnType != "permanent" && Parser::getHttp()->servers[i].returnType != "temporary" && Parser::getHttp()->servers[i].server_directives.find("return")->second.size() != 2)
                throw std::runtime_error("Error: return directive can only have two values: permanent or temporary");
        }

        init_locations(i);
    }
}

Server::~Server()
{
}

Location::Location()
{
    // //std::cout << "Location constructor called" << std::endl;
    this->location_directives.insert(std::pair<std::string, std::vector<std::string> >("path", std::vector<std::string>(1, "/html")));
    this->location_directives.insert(std::pair<std::string, std::vector<std::string> >("return", std::vector<std::string>(1, "200")));
    this->returned = 0;
    this->returnType = "";
    this->returnUrl = "";
    this->path = "/";
    this->Return.first = 200;
    this->Return.second = "OK";
}

void Parser::init_locations(int index)
{
    Server *server = &Parser::getHttp()->servers[index];

    for (size_t i = 0; i < server->locations.size(); i++)
    {

        // root
        if (server->locations[i].location_directives.find("root") != server->locations[i].location_directives.end())
        {
            server->locations[i].root = server->locations[i].location_directives.find("root")->second[0];
            if (server->locations[i].location_directives.find("root")->second.size() != 1)
                throw std::runtime_error("Error: root directive can only have one value");
        }
        else
            server->locations[i].root = server->root;
        // index
        if (server->locations[i].location_directives.find("index") != server->locations[i].location_directives.end())
            server->locations[i].index = server->locations[i].location_directives.find("index")->second;
        else
            server->locations[i].index = server->index;
        // autoindex
        if (server->locations[i].location_directives.find("autoindex") != server->locations[i].location_directives.end())
        {
            if (server->locations[i].location_directives.find("autoindex")->second[0] == "on" || server->locations[i].location_directives.find("autoindex")->second.size() != 1)
                server->locations[i].autoindex = true;
            else if (server->locations[i].location_directives.find("autoindex")->second[0] == "off" || server->locations[i].location_directives.find("autoindex")->second.size() != 1)
                server->locations[i].autoindex = false;
        }
        else
            server->locations[i].autoindex = server->autoindex;
        // client_max_body_size
        if (server->locations[i].location_directives.find("client_max_body_size") != server->locations[i].location_directives.end())
        {
            server->locations[i].client_max_body_size = std::stoi(server->locations[i].location_directives.find("client_max_body_size")->second[0]);
            if (server->locations[i].location_directives.find("client_max_body_size")->second.size() != 1)
                throw std::runtime_error("Error: client_max_body_size directive can only have one value");
        }
        else
            server->locations[i].client_max_body_size = server->client_max_body_size;
        // allowed_methods
        if (server->locations[i].location_directives.find("allowed_methods") != server->locations[i].location_directives.end())
            server->locations[i].allowed_methods = server->locations[i].location_directives.find("allowed_methods")->second;
        else
            server->locations[i].allowed_methods = server->allowed_methods;
    }
}

Location::~Location()
{
}
