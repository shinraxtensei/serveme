#include "inc/parser.hpp"

Lexer *Parser::ptr = nullptr;
Http *Parser::http = nullptr;

#define BLUE "\033[0;34m"
#define RESET "\033[0m"
void generate_dot(Http &http)
{
    std::cout << "digraph G {\n";
    std::cout << "http [shape=box3d ,style=filled];\n";
    int i = 0;
    int j = 0;
    for (auto server : http.servers)
    {
        std::cout << "http -> server" << i << ";\n";
        std::cout << "server" << i << ""
                  << " [shape=box , style=filled, label=\"";
        for (auto directive : server.server_directives)
        {
            std::cout << directive.first << " : ";
            for (auto value : directive.second)
                std::cout << value << " ";
            std::cout << "\\n-----------------------------------------------\\n";
        }
        std::cout << "\"];\n";
        for (auto location : server.locations)
        {
            std::cout << "server" << i << " -> location" << j << ";\n";
            std::cout << "location" << j << ""
                      << " [shape=box , style=filled, label=\"";
            for (auto directive : location.location_directives)
            {
                std::cout << directive.first << " : ";
                for (auto value : directive.second)
                    std::cout << value << " ";
                std::cout << "\\n-----------------------------------------------\\n";
            }
            std::cout << "\"];\n";
            j++;
        }
        i++;
    }
    std::cout << "}\n";
}

int main()
{

    // TODO : hadnle unclosed curly braces and quotes
    // TODO : handle unknown directives
    
    Parser::lex("nginx.conf");

    Parser::parse();

    generate_dot(*Parser::getHttp());
   
    return 0;
}
