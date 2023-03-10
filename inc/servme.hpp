#include "parser.hpp"
#include "core.hpp"
#include "socketWrapper.hpp"
#include "server.hpp"

std::string	generateError(std::string error);

class Servme
{
    static Core *core;

    public:

        static Core *getCore()
        {
            if (core == nullptr)
                core = new Core();
            return core;
        };

};
