#include <SFML/Graphics.hpp>

#include "Core/Program.h"
#include "Core/Services/LogService.h"

int main()
{
    LogService::Initialise();
    Program p;  // create program
    p.Run();    // run program
    LogService::Flush();
}
