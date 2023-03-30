
#include "inc/servme.hpp"

Lexer *Parser::ptr = nullptr;
Http *Parser::http = nullptr;
Core *Servme::core = nullptr;

void generate_dot(Http &http)
{
    std::ofstream Parse_tree;
    Parse_tree.open("parse_tree.dot");
    if (!Parse_tree.is_open())
        throw std::runtime_error("Error opening file");
    Parse_tree << "digraph G {\n";
    Parse_tree << "http [shape=box3d ,style=filled];\n";
    int i = 0;
    int j = 0;
    int k = 0;
    for (auto server : http.servers)
    {
        Parse_tree << "http -> server" << i << ";\n";
        Parse_tree << "server" << i << ""
             << " [shape=box , style=filled, label=\"";
        for (auto directive : server.server_directives)
        {
            Parse_tree << directive.first << " : ";
            for (auto value : directive.second)
                Parse_tree << value << " ";
            Parse_tree << "\\n-----------------------------------------------\\n";
        }
        Parse_tree << "\"];\n";
        for (auto location : server.locations)
        {
            Parse_tree << "server" << i << " -> location" << j << ";\n";
            Parse_tree << "location" << j << ""
                 << " [shape=box , style=filled, label=\"";
            for (auto directive : location.location_directives)
            {
                Parse_tree << directive.first << " : ";
                for (auto value : directive.second)
                    Parse_tree << value << " ";
                Parse_tree << "\\n-----------------------------------------------\\n";
            }
            Parse_tree << "\"];\n";
            for (auto sublocation : location.locations)
            {
                Parse_tree << "location" << j << " -> sublocation" << k << ";\n";
                Parse_tree << "sublocation" << k << ""
                     << " [shape=box , style=filled, label=\"";
                for (auto subdirective : sublocation.location_directives)
                {
                    Parse_tree << subdirective.first << " : ";
                    for (auto value : subdirective.second)
                        Parse_tree << value << " ";
                    Parse_tree << "\\n-----------------------------------------------\\n";
                }
                Parse_tree << "\"];\n";
                k++;
            }
            j++;
        }
        i++;
    }
    Parse_tree << "}\n";
    Parse_tree.close();
}

int main(int argc, char **argv)
{

    if (argc == 1)
    {
        std::cout << BLUE << "---------------------- Serverme  -------------------------" << RESET << std::endl;
    
        Parser::lex("nginx.conf");
        Parser::parse();
        Servme::getCore()->startup();
        Servme::getCore()->handleConnections();
    }


   	if (argc == 2)
    {   
        if (std::string av(argv[1]) ; av == "-h")
        {
            std::cout  << GREEN << "Usage: ./serverme [OPTION]" << RESET << std::endl;
            std::cout << YELLOW << "-h : help" << std::endl;
            std::cout <<  "-t : check the syntax of the configuration file"  << std::endl;
            std::cout <<  "-d : check the syntax of the configuration file and generate a dot file" << RESET << std::endl;   
        }
        else if (std::string av(argv[1]) ; av == "-t")
        {

            Parser::lex("nginx.conf");
            Parser::parse();

            std::cout << GREEN << "Syntax OK" << RESET << std::endl;
        }
        else if (std::string av(argv[1]) ; av == "-d")
        {
            Parser::lex("nginx.conf");
            Parser::parse();
            std::cout << GREEN << "Syntax OK" << RESET << std::endl;
            generate_dot(*Parser::getHttp());
        }
        else
            std::cout << RED << "Usage: ./serverme -h" << RESET << std::endl;
    }
    else
        std::cout << RED << "Usage: ./serverme -h" << RESET << std::endl;
    return 0;
}



