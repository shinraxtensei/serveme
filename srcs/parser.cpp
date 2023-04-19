#include "../inc/parser.hpp"

void Parser::deleteHttp()
{
    if (Parser::http != NULL)
        delete Parser::http;
    Parser::http = NULL;
}

void Parser::deleteLex()
{
    if (Parser::ptr != NULL)
        delete Parser::ptr;
    Parser::ptr = NULL;
}

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
    // //std::cout <<" parse_directives : " << type << std::endl;
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

    if (values.back().size() == 0)
        values.pop_back();

    //* a pair or key and values
    std::pair<std::string, std::vector<std::string> > pair(directive, values);

    //* this is the pair of (iterator , bool ) to check if the pair is inserted or not , if not then its already exists
    std::pair<std::map<std::string, std::vector<std::string> >::iterator, bool> ret;

    if (find(lex()->all_directs.begin(), lex()->all_directs.end(), directive) == lex()->all_directs.end())
        throw std::runtime_error("Error: unkown directive :" + directive);

    if (type == 0)
    {

        Parser::getHttp()->http_directives.insert(pair);

        if (checkDup(Parser::getHttp()->http_directives, pair.first) && directiveDuplicable(directive) == false)
            throw std::runtime_error("Error: duplicate directive :" + directive);
    }

    else if (type == 1)
    {

        Parser::getHttp()->servers.back().server_directives.insert(pair);
        if (checkDup(Parser::getHttp()->servers.back().server_directives, directive) && directiveDuplicable(directive) == false)
            throw std::runtime_error("Error: duplicate directive :" + directive);
    }
    else if (type == 2)
    {

        Parser::getHttp()->servers.back().locations.back().location_directives.insert(pair);

        if (pair.first == "return")
        {
            if (pair.second.size() > 3)
            {
                throw std::runtime_error("Error: return directive must have 2 arguments");
            }
            Parser::getHttp()->servers.back().locations.back().returned = 1;
            Parser::getHttp()->servers.back().locations.back().returnUrl = pair.second[0];
            Parser::getHttp()->servers.back().locations.back().returnType = pair.second[1];
        }
        if (pair.first == "root")
        {
            Parser::getHttp()->servers.back().locations.back().root = pair.second[0];
        }
    }
    else if (type == 3)
    {
        Parser::getHttp()->servers.back().locations.back().locations.back().location_directives.insert(pair);

        if (pair.first == "return")
        {
            if (pair.second.size() > 3)
            {
                throw std::runtime_error("Error: return directive must have 2 arguments");
                // //std::cout << "Error: return directive must have 2 arguments" << std::endl;
                // exit(1);
            }
            Parser::getHttp()->servers.back().locations.back().locations.back().returned = 1;
            Parser::getHttp()->servers.back().locations.back().locations.back().returnUrl = pair.second[0];
            Parser::getHttp()->servers.back().locations.back().locations.back().returnType = pair.second[1];
            if (Parser::getHttp()->servers.back().locations.back().locations.back().returnType != "permanent" && Parser::getHttp()->servers.back().locations.back().locations.back().returnType != "temporary")
                throw std::runtime_error("Error: type not valid: " + Parser::getHttp()->servers.back().locations.back().locations.back().returnType);
        }
        if (pair.first == "root")
        {
            Parser::getHttp()->servers.back().locations.back().locations.back().root = pair.second[0];
        }
    }
    else
        throw std::runtime_error("Error: type not found");
}

void Parser::parse_location(int sublocation)
{
    std::vector<std::string> values;
    if (sublocation == 0)
    {
        Parser::getHttp()->servers.back().locations.push_back(Location());
        Parser::getHttp()->servers.back().locations.back().path = Parser::lex()->next_token(true);
        values.push_back(Parser::getHttp()->servers.back().locations.back().path);
        Parser::getHttp()->servers.back().locations.back().location_directives.insert(std::pair<std::string, std::vector<std::string> >("path", values));
    }
    else if (sublocation == 1)
    {
        Parser::getHttp()->servers.back().locations.back().locations.push_back(Location());
        Parser::getHttp()->servers.back().locations.back().locations.back().path = Parser::lex()->next_token(true);
        values.push_back(Parser::getHttp()->servers.back().locations.back().locations.back().path);
        Parser::getHttp()->servers.back().locations.back().locations.back().location_directives.insert(std::pair<std::string, std::vector<std::string> >("path", values));
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
        throw std::runtime_error("Error: location must be followed by a block");
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
    Parser::lex()->set_input(Parser::lex()->input);
    while (Parser::lex()->next_token(false) != "EOF")
    {
        if (Parser::match("http"))
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
}
