#include "parser/parser.hpp"

Lexer *Parser::ptr = nullptr;
Http *Parser::http = nullptr;


#define BLUE "\033[0;34m"
#define RESET "\033[0m"
void generate_dot(Http& http)
{
    std::cout << "digraph G {\n";
    std::cout << "http [shape=box3d ,style=filled];\n";
    int i = 0 ;
    int j = 0 ;
    for (auto server : http.servers)
    {   
        std::cout << "http -> server" << i << ";\n";
        std::cout << "server" << i << "" << " [shape=box , style=filled, label=\"";
        for(auto directive : server.server_directives)
        {
            std::cout   << directive.first  << " : ";
            for(auto value : directive.second)
                std::cout << value << " ";
            std::cout << "\\n-----------------------------------------------\\n";
        }
        std::cout << "\"];\n";
        for (auto location : server.locations)
        {
            std::cout << "server" << i << " -> location" << j << ";\n";
            std::cout << "location" << j  << "" << " [shape=box , style=filled, label=\"";
            for(auto directive : location.location_directives)
            {
                std::cout   << directive.first  << " : ";
                for(auto value : directive.second)
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

    Parser::lex("nginx.conf");

    Parser::parse();

    std::cout  << std::endl<< "********* DOT  *********" << std::endl;

    generate_dot(*Parser::getHttp());
    // std::cout << "digraph G {\n";
    // for (const auto& [key, values] : Parser::getHttp()->http_directives) {
    //     std::cout << "  " << key << " -> {";
    //     for (const auto& value : values) {
    //         std::cout << " \"" << value << "\"";
    //     }
    //     std::cout << " }\n";
    // }
    // std::cout << "}\n";


    // for (auto it : Parser::getHttp()->servers[0].server_directives)
    // {
    //     std::cout << it.first << " : ";
    //     for (auto i : it.second)
    //         std::cout << i << " ";
    //     std::cout << std::endl;
    // }
    // std::cout << "------------------" << std::endl;
    // std::cout << "html direcitves :  " << std::endl;
    // for (auto i : Parser::getHttp()->http_directives)
    // {
    //     std::cout << i.first << " : ";
    //     for (auto j : i.second)
    //         std::cout << j << " ";
    //     std::cout << std::endl;
    // }

    // *********  *********



    // for (auto i : Parser::getHttp()->servers)
    // {
    //     std::cout << "server :" << std::endl;
        // for (auto j : Parser::getHttp()->servers[0].server_directives)
        // {
        //     std::cout << j.first << " : ";
        //     for (auto k : j.second)
        //         std::cout << k << " ";
        //     std::cout << std::endl;
        // }
    //     for (auto j : i.locations)
    //     {
    //         std::cout << "location :" << std::endl;
    //         for (auto k : j.location_directives)
    //         {
    //             std::cout << k.first << " : ";
    //             for (auto l : k.second)
    //                 std::cout << l << " ";
    //             std::cout << std::endl;
    //         }
    //     }
    // }

    // print the content of the http direcives
    // for (auto i : Parser::getHttp()->servers[1].server_directives)
    // {
    //     std::cout << i.first << " : ";
    //     for (auto j : i.second)
    //         std::cout << j << " ";
    //     std::cout << std::endl;
    // }
    // for (auto i : Parser::lex()->lines)
    // std::cout << "line :" << i << std::endl;
    // std::string input;
    // while (input != "EOF")
    // input = Parser::lex()->next_token(Parser::lex()->input);
    // for (auto i : Parser::lex()->tokens)
    //     std::cout << i << std::endl;
    // std::vector<std::string> lines = Parser::lex()->lines;
    // std::cout << Parser::lex()->lines[2] << std::endl;

    // for(auto line : lines)
    // {
    //     Parser::lex()->set_input(line);
    //     std::string token = Parser::lex()->next_token();
    //     while (token != "EOF")
    //     {
    //         std::cout << token << std::endl;
    //         token = Parser::lex()->next_token();
    //     }

    // }

    // std::cout << Parser::lex()->next_token(lines[2]) << std::endl;
    // std::cout << Parser::lex()->next_token(lines[2]) << std::endl;

    return 0;
}
