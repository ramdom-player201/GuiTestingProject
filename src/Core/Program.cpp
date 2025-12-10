#include "Program.h"
#include <iostream>

#include "ConsoleColours.h"
#include "Services.h"

void Program::Run()
{
	//ColouredConsole::print(ConsoleColour::GREEN, "Starting ");
	//ColouredConsole::print(ConsoleColour::CYAN, "Program",true);

	std::cout << ConsoleColours::getColourCode(AnsiColours::GREEN) << "Starting "
		<< ConsoleColours::getColourCode(AnsiColours::CYAN)<< "program\n";

	Services::initialise();
	//auto windowManager = dynamic_cast<WindowManager*>(Services::getService(ServiceType::WindowManager));
	//auto windowManager2 = dynamic_cast<WindowManager*>(Services::getService(ServiceType::WindowManager));
	auto commandConsole = dynamic_cast<CommandConsole*>(Services::getService(ServiceType::CommandConsole));
	auto objectManager = dynamic_cast<ObjectManager*>(Services::getService(ServiceType::ObjectManager));

	WindowManager::debugMode = true;

	WindowManager::countWindows();
	WindowManager::createWindow("Window 1");
	WindowManager::createWindow("Window 2");
	WindowManager::countWindows();

	WindowManager::createWindow("Window 33");

	WindowManager::debugMode = false;
	while (WindowManager::countWindows() > 0){
	// program should get call console to get commands
		bool commandLinePause = WindowManager::updateWindows();
		if (commandLinePause) {
			commandConsole->parseCommand();
		}
	}
}

Program::Program()
{
	// Initialise GLFW
	if (!glfwInit()) {
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Program > "
			<< ConsoleColours::getColourCode(AnsiColours::YELLOW) << "[Constructor] :: "
			<< ConsoleColours::getColourCode(AnsiColours::RED) << "GLFW failed to initialise\n";
		std::cerr << "Failed to initialize GLFW" << std::endl;
	}
	else {
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Program > "
			<< ConsoleColours::getColourCode(AnsiColours::YELLOW) << "[Constructor] :: "
			<< ConsoleColours::getColourCode(AnsiColours::CYAN) << "GLFW initialised successfully\n";
	}

}

Program::~Program()
{
	std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Program > "
		<< ConsoleColours::getColourCode(AnsiColours::YELLOW) << "[Destructor] :: "
		<< ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "Program closing\n";
	//glfwTerminate(); // Note: in the event of multiple program creations,
					   //       ensure program keeps track of how many exist so this cannot be deinitialised mid-execution
}
