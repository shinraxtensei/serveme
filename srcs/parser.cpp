#include "../inc/parser.hpp"


bool Parser::match(std::string token)
{
    if (token == Parser::lex()->next_token(false))
    {
        Parser::lex()->next_token(true);
        return true;
    }
    return false;
}

void Parser::parse_directives(int type)
{
    // std::cout <<" parse_directives : " << type << std::endl;
    std::string directive = Parser::lex()->next_token(true);
    std::vector<std::string> values;
    std::string value;
    while (1)
    {
        value = Parser::lex()->next_token(true);
        if (value.back() == ';')
        {
            value.pop_back();
            values.push_back(value);
            break;
        }
        values.push_back(value);
    }

    //* a pair or key and values
    std::pair<std::string, std::vector<std::string> > pair(directive, values);
    
    //* this is the pair of (iterator , bool ) to check if the pair is inserted or not , if not then its already exists
    std::pair<std::map<std::string, std::vector<std::string> >::iterator, bool> ret;

    if ( find(lex()->all_directs.begin(),lex()->all_directs.end(), directive) == lex()->all_directs.end())
    {
        std::cout << directive << std::endl;
        std::cout << "Error: directive not found" << std::endl;
        exit(1);
    }


    if (type == 0)
    {
        Parser::getHttp()->http_directives[directive] = values;
    }

    else if (type == 1)
    {
        Parser::getHttp()->servers.back().server_directives[directive] = values;
    }
    else if (type == 2)
    {
        Parser::getHttp()->servers.back().locations.back().location_directives[directive] = values;
        if (pair.first == "return")
        {
                Parser::getHttp()->servers.back().locations.back().Return.first = std::stoi(pair.second[0]);
                Parser::getHttp()->servers.back().locations.back().Return.second = pair.second[1];
        }
    }
    else
        std::cout << "Error: type not found" << std::endl;
}

void Parser::parse_location(int sublocation)
{
    std::vector<std::string> values;
    if (sublocation == 0)
    {
        Parser::getHttp()->servers.back().locations.push_back(Location());
        Parser::getHttp()->servers.back().locations.back().path = Parser::lex()->next_token(true);
        values.push_back(Parser::getHttp()->servers.back().locations.back().path);
        Parser::getHttp()->servers.back().locations.back().location_directives.insert(std::pair<std::string, std::vector<std::string> > ("path" , values ));

    }
    else if (sublocation == 1)
    {
        Parser::getHttp()->servers.back().locations.back().locations.push_back(Location());
        Parser::getHttp()->servers.back().locations.back().locations.back().path = Parser::lex()->next_token(true);
        values.push_back(Parser::getHttp()->servers.back().locations.back().locations.back().path);
        Parser::getHttp()->servers.back().locations.back().locations.back().location_directives.insert(std::pair<std::string, std::vector<std::string> > ("path" , values ));

    }
    if (Parser::match("{"))
    {
        while (1)
        {
            if (Parser::match("location"))
                parse_location(1);
            else if (Parser::match("}"))
                break;
            else
                parse_directives(2);
        }
    }
    else
    {
        std::cout << "Error: location must be followed by a block" << std::endl;
        exit(1);
    }
}

void Parser::parse_server()
{
    Parser::getHttp()->servers.push_back(Server());
    if (Parser::match("{"))
    {
        while (1)
        {
            if (Parser::match("location"))
                parse_location(0);
            else if (Parser::match("}"))
                break;
            else
                parse_directives(1);
        }
    }
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
        
        // index
        if (Parser::getHttp()->servers[i].server_directives.find("index") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].index = Parser::getHttp()->servers[i].server_directives["index"];


        // error_page
        if (Parser::getHttp()->servers[i].server_directives.find("error_page") != Parser::getHttp()->servers[i].server_directives.end())
        {
            Parser::getHttp()->servers[i].error_page.first = std::stoi(Parser::getHttp()->servers[i].server_directives["error_page"][0]);
            Parser::getHttp()->servers[i].server_directives["error_page"].erase(Parser::getHttp()->servers[i].server_directives["error_page"].begin());
            Parser::getHttp()->servers[i].error_page.second = Parser::getHttp()->servers[i].server_directives["error_page"];
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

        // client_max_body_size
        if (Parser::getHttp()->servers[i].server_directives.find("client_max_body_size") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].client_max_body_size = std::stoi(Parser::getHttp()->servers[i].server_directives["client_max_body_size"][0]);



        // allowed_methods
        if  (Parser::getHttp()->servers[i].server_directives.find("allowed_methods") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].allowed_methods = Parser::getHttp()->servers[i].server_directives["allowed_methods"];
    }
}





void Parser::parse()
{

    // TODO : handle a bug (when the last directive and ;  are seperated by a space the ; is taken as a token by itself) -> this makes the parser return an error of more that one value for a directive that can only have one value
    
    Parser::lex()->set_input(Parser::lex()->input);
    while (Parser::lex()->next_token(false) != "EOF")
    {
        if (Parser::match("http") )
        {
            if (Parser::match("{"))
            {
                while (1)
                {
                    if (Parser::match("server"))
                        parse_server();
                    else if (Parser::match("}"))
                        break;
                    else
                        parse_directives(0);
                }
            }
        }
        else if (Parser::match("server"))
        {
            if (Parser::match("{"))
            {
                while (1)
                {
                    if (Parser::match("location"))
                        parse_location(0);
                    else if (Parser::match("}"))
                        break;
                    else
                        parse_directives(1);
                }
            }
        }
        else
            parse_directives(0);
    }

    Parser::init_servers();

    for (auto server  : Parser::getHttp()->servers)
    {
  
        std::cout << "---------------------------------" << std::endl;
        std::cout << "server_name : " << server.server_name << std::endl;
        std::cout << "root : " << server.root << std::endl;
        std::cout << "index : " << server.index[0] << std::endl;
        std::cout << "error_page : "<< server.error_page.first << " " << server.error_page.second[0]  << std::endl;
        std::cout << "autoindex : " << server.autoindex << std::endl;
        std::cout << "client_max_body_size : " << server.client_max_body_size << std::endl;
        std::cout << "allowed_methods : " << server.allowed_methods[0] << std::endl;
        std::cout << "ip : " << server.ipPort.first << std::endl;
        std::cout << "port : " << server.ipPort.second << std::endl;
        std::cout << "---------------------------------" << std::endl;

    }
}
