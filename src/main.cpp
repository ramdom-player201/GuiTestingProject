#include <SFML/Graphics.hpp>

#include "Core/Program.h"
#include "Core/Services/LogService.h"
#include "Core/Services/FileManager.h"
#include <iostream>

int main()
{
    //std::cout << "Log size: " << LogService::GetCurrentDataUsage() << std::endl;

    // quick tests
    LogService::Log(LogType::TEST, "main", "main", "Running temporary file manager tests.");
    FileManager::ValidatePath("Place");
    FileManager::ValidatePath("Place/A/B");
    FileManager::ValidatePath("Place/A/B/../D");
    FileManager::ValidatePath("Place/../../../../A");
    FileManager::ValidatePath("HyperSphere/TextFile.txt");
    FileManager::ValidatePath("Place/../../../../Test.txt");
    FileManager::ValidatePath("HyperCube/Cube.cube.txt");
    FileManager::ValidatePath("HyperSphere/Cube.txt/A");

    // main code
    LogService::Initialise();
    Program p;  // create program
    p.Run();    // run program
    LogService::Flush();

    //std::cout << "Log size: " << LogService::GetCurrentDataUsage() << std::endl;
}