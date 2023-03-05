#include "inc/parser.hpp"
#include "inc/core.hpp"
#include "inc/socketWrapper.hpp"
#include "inc/server.hpp"
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
            j++;
        }
        i++;
    }
    Parse_tree << "}\n";
    Parse_tree.close();
}

int main()
{

    std::cout << BLUE << "---------------------- Serverme  -------------------------" << RESET << std::endl;
    
    Parser::lex("nginx.conf");
    Parser::parse();
    generate_dot(*Parser::getHttp());
        
    Servme::getCore()->startup();
    Servme::getCore()->handleConnections();



 
    return 0;
}
