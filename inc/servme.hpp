#include "parser.hpp"
#include "core.hpp"
#include "socketWrapper.hpp"
#include "server.hpp"


class Servme
{
    static Core *core;
    // static Parser *parser;

    public:

        static Core *getCore()
        {
            if (core == nullptr)
                core = new Core();
            return core;
        };

};
