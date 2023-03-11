#include "../inc/servme.hpp"

Core *Servme::getCore()
{
    if (Servme::core == nullptr)
        Servme::core = new Core();
    return Servme::core;
};