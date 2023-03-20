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
        ret = Parser::getHttp()->http_directives.insert(pair);
        if (ret.second == false)
        {
            std::cout << "Error: directive already exists" << std::endl;
            exit(1);
        }
    }

    else if (type == 1)
    {
        if (pair.first == "listen" )
        {
            if (pair.second.size() > 1  && pair.second[1] == ";")
            {
                std::cout << "Error: listen directive can only have one value" << std::endl;
                exit(1);
            }
            else if (pair.second[0].find(":") != std::string::npos)
            {
                Parser::getHttp()->servers.back().ipPort.first = pair.second[0].substr(0, pair.second[0].find(":"));
                Parser::getHttp()->servers.back().ipPort.second = std::stoi(pair.second[0].substr(pair.second[0].find(":") + 1));
            }
            else
            {
                Parser::getHttp()->servers.back().ipPort.first = "NONE";
                Parser::getHttp()->servers.back().ipPort.second = std::stoi(pair.second[0]);
            }

        }
        else if (pair.first == "server_name" )
        {
            if (pair.second.size() > 1 && pair.second[1] == ";")
            {
                std::cout << "Error: server_name directive can only have one value" << std::endl;
                exit(1);
            }
            else
                Parser::getHttp()->servers.back().server_name = pair.second[0];
        }
        

        ret = Parser::getHttp()->servers.back().server_directives.insert(pair);
        if (ret.second == false)
        {
            std::cout << "Error: directive already exists" << std::endl;
            exit(1);
        }
    }
    else if (2)
    {
        if (pair.first == "return")
        {
                Parser::getHttp()->servers.back().locations.back().Return.first = std::stoi(pair.second[0]);
                Parser::getHttp()->servers.back().locations.back().Return.second = pair.second[1];
        }

        ret = Parser::getHttp()->servers.back().locations.back().location_directives.insert(pair);
        if (ret.second == false)
        {
            std::cout << "Error: directive already exists" << std::endl;
            exit(1);
        }
    }
    else
        std::cout << "Error: type not found" << std::endl;
}

void Parser::parse_location()
{

    Parser::getHttp()->servers.back().locations.push_back(Location());
    while(!Parser::match("{"))
        Parser::lex()->next_token(true);
    while (1)
    {
        if (Parser::match("location"))
            Parser::parse_location();
        else if (Parser::match("}"))
            return;
        else
            Parser::parse_directives(2);
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
                parse_location();
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
        // root
        if (Parser::getHttp()->servers[i].server_directives.find("root") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].root = Parser::getHttp()->servers[i].server_directives["root"][0];
        else if (Parser::getHttp()->http_directives.find("root") != Parser::getHttp()->http_directives.end())
            Parser::getHttp()->servers[i].root = Parser::getHttp()->http_directives["root"][0];
        else
            Parser::getHttp()->servers[i].root = "../html"; // default value
        
        // index
        if (Parser::getHttp()->servers[i].server_directives.find("index") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].index = Parser::getHttp()->servers[i].server_directives["index"];
        else if  (Parser::getHttp()->http_directives.find("index") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].index = Parser::getHttp()->http_directives["index"];
        else
            Parser::getHttp()->servers[i].index[0] = "index.html"; // default value

        // error_page
        if (Parser::getHttp()->servers[i].server_directives.find("error_page") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].error_page = Parser::getHttp()->servers[i].server_directives["error_page"];
        else if (Parser::getHttp()->http_directives.find("error_page") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].error_page = Parser::getHttp()->http_directives["error_page"];
        else
            Parser::getHttp()->servers[i].error_page[0] = "error.html"; // default value

        // autoindex
        if (Parser::getHttp()->servers[i].server_directives.find("autoindex") != Parser::getHttp()->servers[i].server_directives.end())
        {
            if (Parser::getHttp()->servers[i].server_directives["autoindex"][0] == "on")
                Parser::getHttp()->servers[i].autoindex = true;
            else
                Parser::getHttp()->servers[i].autoindex = false;
        }
        else if (Parser::getHttp()->http_directives.find("autoindex") != Parser::getHttp()->http_directives.end())
        {
            if (Parser::getHttp()->http_directives["autoindex"][0] == "on")
                Parser::getHttp()->servers[i].autoindex = true;
            else
                Parser::getHttp()->servers[i].autoindex = false;
        }
        else
            Parser::getHttp()->servers[i].autoindex = false; // default value



        // client_max_body_size
        if (Parser::getHttp()->servers[i].server_directives.find("client_max_body_size") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].client_max_body_size = std::stoi(Parser::getHttp()->servers[i].server_directives["client_max_body_size"][0]);
        else if (Parser::getHttp()->http_directives.find("client_max_body_size") != Parser::getHttp()->http_directives.end())
            Parser::getHttp()->servers[i].client_max_body_size = std::stoi(Parser::getHttp()->http_directives["client_max_body_size"][0]);
        else
            Parser::getHttp()->servers[i].client_max_body_size = 1000000; // default value


        // allowed_methods
        if  (Parser::getHttp()->servers[i].server_directives.find("allowed_methods") != Parser::getHttp()->servers[i].server_directives.end())
            Parser::getHttp()->servers[i].allowed_methods = Parser::getHttp()->servers[i].server_directives["allowed_methods"];
        else if ((Parser::getHttp()->http_directives.find("allowed_methods") != Parser::getHttp()->http_directives.end()))
            Parser::getHttp()->servers[i].allowed_methods = Parser::getHttp()->http_directives["allowed_methods"];
        else
            Parser::getHttp()->servers[i].allowed_methods = {"GET", "HEAD", "POST", "PUT", "DELETE", "OPTIONS", "TRACE", "CONNECT", "PATCH"}; // default value
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
                        parse_location();
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
}
