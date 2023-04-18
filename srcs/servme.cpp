#include "../inc/servme.hpp"

Core *Servme::getCore()
{
    if (Servme::core == NULL)
        Servme::core = new Core();
    return Servme::core;
};

void Servme::deleteCore()
{
    if (Servme::core != NULL)
        delete Servme::core;
    Servme::core = NULL;
};