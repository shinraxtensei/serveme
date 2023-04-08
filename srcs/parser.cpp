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

bool checkDup(std::multimap<std::string, std::vector<std::string> > map, std::string key)
{
    // if (map.equal_range(key).first == map.equal_range(key).second)
        // return false;
    // if ((map.equal_range(key).second - map.equal_range(key).first) > 1)
        // return true;
    int counter = 0;
    for (std::multimap<std::string, std::vector<std::string> >::iterator it = map.begin(); it != map.end(); it++)
    {
        if (it->first == key)
            counter++;
    }
    if (counter > 1)
        return true;
    return false;
}

bool directiveDuplicable(std::string directive)
{
    if (directive == "error_page")
        return true;
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
        // Parser::getHttp()->http_directives[directive] = values;
        Parser::getHttp()->http_directives.insert(pair);

        // std::cout << RED << Parser::getHttp()->http_directives.find(directive)->second[0] << RESET << std::endl;

        if (checkDup(Parser::getHttp()->http_directives, pair.first) && directiveDuplicable(directive) == false )
        {   

            std::cout << "Error: duplicate directive : " << std::endl;
            std::cout << directive << " with a value of : " << pair.second[0]  << std::endl;
            exit(1);
        }
        

    }

    else if (type == 1)
    {
        // Parser::getHttp()->servers.back().server_directives[directive] = values;
        Parser::getHttp()->servers.back().server_directives.insert(pair);
        if (checkDup(Parser::getHttp()->servers.back().server_directives, directive) && directiveDuplicable(directive) == false)
        {
            std::cout << "Error: duplicate directive : " << std::endl;
            std::cout << directive << " with a value of : " << values[0]  << std::endl;
            exit(1);
        }
    }
    else if (type == 2)
    {
        // Parser::getHttp()->servers.back().locations.back().location_directives[directive] = values;
        Parser::getHttp()->servers.back().locations.back().location_directives.insert(pair);
        // if (checkDup(Parser::getHttp()->servers.back().locations.back().location_directives, directive) && directiveDuplicable(directive) == false)
        // {
        //     std::cout << "Error: duplicate directive: " << directive << std::endl;
        //     exit(1);
        // }
        if (pair.first == "return")
        {       
                if (pair.second.size() > 3)
                {
                    std::cout << "Error: return directive must have 2 arguments" << std::endl;
                    exit(1);
                }
                Parser::getHttp()->servers.back().locations.back().returned = 1;
                Parser::getHttp()->servers.back().locations.back().returnUrl = pair.second[0];
                Parser::getHttp()->servers.back().locations.back().returnType = pair.second[1];

                // Parser::getHttp()->servers.back().locations.back().Return.first = std::stoi(pair.second[0]);
                // Parser::getHttp()->servers.back().locations.back().Return.second = pair.second[1];
        }
        if (pair.first == "root")
        {
            Parser::getHttp()->servers.back().locations.back().root = pair.second[0];
        }
    }
    else if (type == 3)
    {
        // Parser::getHttp()->servers.back().locations.back().locations.back().location_directives[directive] = values;
        Parser::getHttp()->servers.back().locations.back().locations.back().location_directives.insert(pair);
        
        // if (checkDup(Parser::getHttp()->servers.back().locations.back().locations.back().location_directives, directive) && directiveDuplicable(directive) == false )
        // {
        //     std::cout << "Error: duplicate directive :" << directive << std::endl;
        //     exit(1);
        // }
        if (pair.first == "return")
        {
                if (pair.second.size() > 3)
                {
                    std::cout << "Error: return directive must have 2 arguments" << std::endl;
                    exit(1);
                }
                Parser::getHttp()->servers.back().locations.back().locations.back().returned = 1;
                Parser::getHttp()->servers.back().locations.back().locations.back().returnUrl = pair.second[0];
                Parser::getHttp()->servers.back().locations.back().locations.back().returnType = pair.second[1];
                
                // Parser::getHttp()->servers.back().locations.back().locations.back().Return.first = std::stoi(pair.second[0]);
                // Parser::getHttp()->servers.back().locations.back().locations.back().Return.second = pair.second[1];
        }
        if (pair.first == "root")
        {
            Parser::getHttp()->servers.back().locations.back().locations.back().root = pair.second[0];
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
            {
                if (sublocation == 0)
                    parse_directives(2);
                else if (sublocation == 1)
                    parse_directives(3);
            }
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
    Parser::init_http();
    Parser::init_servers();

    // for (auto server  : Parser::getHttp()->servers)
    // {
    //     std::cout << "---------------------------------" << std::endl;
    //     std::cout << "server_name : " << server.server_name << std::endl;
    //     std::cout << "root : " << server.root << std::endl;
    //     std::cout << "index : " << server.index[0] << std::endl;
    //     // std::cout << "error_page : "<< server.error_page.first ;
    //     for(auto page : server.error_page)
    //     {
    //         std::cout << "error_page : "<< page.first << " "  << std::endl;
    //         for(auto p : page.second)
    //             std::cout << p << std::endl;
    //     }

    //     // std::cout << "error_page : "<< server.error_page.first << " " << server.error_page.second[0]  << std::endl;
    //     std::cout << "autoindex : " << server.autoindex << std::endl;
    //     std::cout << "client_max_body_size : " << server.client_max_body_size << std::endl;
    //     for(auto method : server.allowed_methods)
    //         std::cout << "allowed_methods : " << method << std::endl;
    //     // std::cout << "allowed_methods : " << server.allowed_methods[0] << std::endl;
    //     std::cout << "ip : " << server.ipPort.first << std::endl;
    //     std::cout << "port : " << server.ipPort.second << std::endl;
    //     std::cout << "---------------------------------" << std::endl;

    //     for (auto location : server.locations)
    //     {
    //         std::cout  << "location " <<std::endl; 
    //         std::cout << BLUE << "root" << RESET << " : " << location.root << std::endl;
    //         std::cout<< BLUE << "location path :"<< RESET <<  location.path << std::endl;
    //         std::cout << BLUE<< "return :" << RESET <<  location.Return.first << " " << location.Return.second << std::endl;
    //     }
    // }
}
