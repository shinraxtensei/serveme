#include "../inc/core.hpp"




// ! : not used for now but can have important impolementations , so not deleting it for now 



Http::Http()
{
    // std::cout << "Http constructor called : " << this << std::endl;
    this->http_directives.insert(std::pair<std::string, std::vector<std::string> >("root", std::vector<std::string> (1 , "/html")));
    this->http_directives.insert(std::pair<std::string, std::vector<std::string> >("", std::vector<std::string> (1, "index.html")));
    this->http_directives.insert(std::pair<std::string, std::vector<std::string> >("error_page", std::vector<std::string> (1 , "404")));
    this->http_directives.insert(std::pair<std::string, std::vector<std::string> >("autoindex", std::vector<std::string> (1 , "off")));
    this->http_directives.insert(std::pair<std::string, std::vector<std::string> >("client_max_body_size", std::vector<std::string> (1 , "1024")));
    this->http_directives.insert(std::pair<std::string, std::vector<std::string> >("allowed_methods", std::vector<std::string> ( 1 , "GET")));

    this->allowed_methods.push_back("GET");
    this->allowed_methods.push_back("POST");
    this->allowed_methods.push_back("DELETE");

    this->autoindex = false;
    this->client_max_body_size = 1024;
    this->error_page = std::pair<int, std::vector<std::string> > (404, std::vector<std::string> (1, "404.html"));
    this->index.push_back("");
    this->root = "/html";


    // this->http_directives["root"] = std::vector<std::string> (1 , "/html") ;
    // this->http_directives["index"] = std::vector<std::string> (1, "index.html");
    // this->http_directives["error_page"] = std::vector<std::string> (1 , "404");
    // this->http_directives["autoindex"] = std::vector<std::string> (1 , "off");
    // this->http_directives["client_max_body_size"] = std::vector<std::string> (1 , "1");
    // this->http_directives["allowed_methods"] = std::vector<std::string> ( 1 , "GET");
}

void Parser::init_http()
{
    if (Parser::getHttp()->http_directives.find("root") != Parser::getHttp()->http_directives.end())
        Parser::getHttp()->root = Parser::getHttp()->http_directives["root"][0];
    if (Parser::getHttp()->http_directives.find("") != Parser::getHttp()->http_directives.end())
        Parser::getHttp()->index = Parser::getHttp()->http_directives["index"];
    if (Parser::getHttp()->http_directives.find("client_max_body_size") != Parser::getHttp()->http_directives.end())
        Parser::getHttp()->client_max_body_size = atoi(Parser::getHttp()->http_directives["client_max_body_size"][0].c_str());
    if (Parser::getHttp()->http_directives.find("error_page") != Parser::getHttp()->http_directives.end())
    {
        Parser::getHttp()->error_page.first = atoi(Parser::getHttp()->http_directives["error_page"][0].c_str());
        Parser::getHttp()->http_directives["error_page"].erase(Parser::getHttp()->http_directives["error_page"].begin());
        Parser::getHttp()->error_page.second = Parser::getHttp()->http_directives["error_page"];
    }
    if (Parser::getHttp()->http_directives.find("autoindex") != Parser::getHttp()->http_directives.end())
    {
        if (Parser::getHttp()->http_directives["autoindex"][0] == "on")
            Parser::getHttp()->autoindex = true;
        else if (Parser::getHttp()->http_directives["autoindex"][0] == "off")
            Parser::getHttp()->autoindex = false;
        else
        {
            std::cout << "Error: autoindex can only be on or off" << std::endl;
            exit(1);
        }
    }
    if (Parser::getHttp()->http_directives.find("allowed_methods") != Parser::getHttp()->http_directives.end())
        Parser::getHttp()->allowed_methods = Parser::getHttp()->http_directives["allowed_methods"];
}

Http::~Http()
{
    // std::cout << "Http destructor" << std::endl;
}








Server::Server()
{
    // std::cout << "Server constructor called" << std::endl;


    this->server_directives.insert(std::pair<std::string, std::vector<std::string> >("root", std::vector<std::string> (1 , "/html")));
    this->server_directives.insert(std::pair<std::string, std::vector<std::string> >("", std::vector<std::string> (1, "index.html")));
    this->server_directives.insert(std::pair<std::string, std::vector<std::string> >("error_page", std::vector<std::string> (1 , "404")));
    this->server_directives.insert(std::pair<std::string, std::vector<std::string> >("autoindex", std::vector<std::string> (1 , "off")));
    this->server_directives.insert(std::pair<std::string, std::vector<std::string> >("client_max_body_size", std::vector<std::string> (1 , "1024")));
    this->server_directives.insert(std::pair<std::string, std::vector<std::string> >("allowed_methods", std::vector<std::string> ( 1 , "GET")));

    this->server_directives.insert(std::pair<std::string, std::vector<std::string> >("listen", std::vector<std::string> (1 , "6969")));
    this->server_directives.insert(std::pair<std::string, std::vector<std::string> >("server_name", std::vector<std::string> (1 , "localhost")));

    this->ipPort.first = "NONE";
    this->ipPort.second = 6969;

    this->server_name = "localhost";
    // this->server_directives["listen"] = std::vector<std::string> (1 , "8080");
    // this->server_directives["server_name"] = std::vector<std::string> (1 , "localhost");
}

void Parser::init_servers()
{
    for (size_t i = 0; i < Parser::getHttp()->servers.size(); i++)
    {
        // server_name
        if (Parser::getHttp()->servers[i].server_directives.find("server_name") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].server_name = Parser::getHttp()->servers[i].server_directives["server_name"][0];

        // listen
        if (Parser::getHttp()->servers[i].server_directives.find("listen") != Parser::getHttp()->servers[i].server_directives.end())
        {
            std::pair <std::string , std::vector<std::string> > listenDirective = *Parser::getHttp()->servers[i].server_directives.find("listen");
            if (listenDirective.second.size() > 1 && listenDirective.second[1] == ";")
            {
                std::cout << "Error: listen directive can only have one value" << std::endl;
                exit(1);
            }
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
            Parser::getHttp()->servers[i].root = Parser::getHttp()->servers[i].server_directives["root"][0];
        else
           Parser::getHttp()->servers[i].root = Parser::getHttp()->root;
        
        // index
        if (Parser::getHttp()->servers[i].server_directives.find("index") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].index = Parser::getHttp()->servers[i].server_directives["index"];
        else
            Parser::getHttp()->servers[i].index = Parser::getHttp()->index;


        // error_page
        if (Parser::getHttp()->servers[i].server_directives.find("error_page") != Parser::getHttp()->servers[i].server_directives.end())
        {
            Parser::getHttp()->servers[i].error_page.first = atoi(Parser::getHttp()->servers[i].server_directives["error_page"][0].c_str());
            Parser::getHttp()->servers[i].server_directives["error_page"].erase(Parser::getHttp()->servers[i].server_directives["error_page"].begin());
            Parser::getHttp()->servers[i].error_page.second = Parser::getHttp()->servers[i].server_directives["error_page"];
        }
        else
        {
            Parser::getHttp()->servers[i].error_page.first = Parser::getHttp()->error_page.first;
            Parser::getHttp()->servers[i].error_page.second = Parser::getHttp()->error_page.second;
        }

        // autoindex
        if (Parser::getHttp()->servers[i].server_directives.find("autoindex") != Parser::getHttp()->servers[i].server_directives.end())
        {
            // std::cout << RED << "autoindex: " << Parser::getHttp()->servers[i].server_directives["autoindex"][1] << RESET <<std::endl;
            if (Parser::getHttp()->servers[i].server_directives["autoindex"][0] == "on")
                Parser::getHttp()->servers[i].autoindex = true;
            else
                Parser::getHttp()->servers[i].autoindex = false;
        }
        else
            Parser::getHttp()->servers[i].autoindex = Parser::getHttp()->autoindex;

        // client_max_body_size
        if (Parser::getHttp()->servers[i].server_directives.find("client_max_body_size") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].client_max_body_size = std::stoi(Parser::getHttp()->servers[i].server_directives["client_max_body_size"][0]);
        else
            Parser::getHttp()->servers[i].client_max_body_size = Parser::getHttp()->client_max_body_size;


        // allowed_methods
        if  (Parser::getHttp()->servers[i].server_directives.find("allowed_methods") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].allowed_methods = Parser::getHttp()->servers[i].server_directives["allowed_methods"];
        else
            Parser::getHttp()->servers[i].allowed_methods = Parser::getHttp()->allowed_methods;

        init_locations(i);
    }
}




Server::~Server()
{
    // std::cout << "Server destructor" << std::endl;
}







Location::Location()
{
    // std::cout << "Location constructor called" << std::endl;
    this->location_directives.insert(std::pair<std::string, std::vector<std::string> >("path", std::vector<std::string> (1 , "/html")));
    this->location_directives.insert(std::pair<std::string, std::vector<std::string> >("return", std::vector<std::string> (1 , "200")));
    this->returned = 0;
    this->returnType = "";
    this->returnUrl = "";
    this->path = "/";
    this->Return.first = 200;
    this->Return.second = "OK";
    
    // this->location_directives["path"] = std::vector<std::string> (1 , "/html");
    // this->location_directives["return"] = std::vector<std::string> (1 , "200");
}

void Parser::init_locations(int index)
{
    Server *server = &Parser::getHttp()->servers[index];


    for (size_t i = 0; i < server->locations.size(); i++)
    {
        // //return
        // if (server->locations[i].location_directives.find("return") != server->locations[i].location_directives.end())
        // {
        //     server->locations[i].Return.first = atoi(server->locations[i].location_directives["return"][0].c_str());
        //     server->locations[i].Return.second = server->locations[i].location_directives["return"][1];
        // }
        // else
        // {
        //     server->locations[i].Return.first = server->Return.first;
        //     server->locations[i].Return.second = server.RReturn.second;
        // }
        //root
        if (server->locations[i].location_directives.find("root") != server->locations[i].location_directives.end())
            server->locations[i].root = server->locations[i].location_directives["root"][0];
        else
            server->locations[i].root = server->root;
        //index
        if (server->locations[i].location_directives.find("index") != server->locations[i].location_directives.end())
            server->locations[i].index = server->locations[i].location_directives["index"];
        else
            server->locations[i].index = server->index;
        //autoindex
        if (server->locations[i].location_directives.find("autoindex") != server->locations[i].location_directives.end())
        {
            if (server->locations[i].location_directives["autoindex"][0] == "on")
                server->locations[i].autoindex = true;
            else
                server->locations[i].autoindex = false;
        }
        else
            server->locations[i].autoindex = server->autoindex;
        //client_max_body_size
        if (server->locations[i].location_directives.find("client_max_body_size") != server->locations[i].location_directives.end())
            server->locations[i].client_max_body_size = std::stoi(server->locations[i].location_directives["client_max_body_size"][0]);
        else
            server->locations[i].client_max_body_size = server->client_max_body_size;
        //allowed_methods
        if  (server->locations[i].location_directives.find("allowed_methods") != server->locations[i].location_directives.end())
            server->locations[i].allowed_methods = server->locations[i].location_directives["allowed_methods"];
        else
            server->locations[i].allowed_methods = server->allowed_methods;
        
    }


}


Location::~Location()
{
    // std::cout << "Location destructor" << std::endl;
}
