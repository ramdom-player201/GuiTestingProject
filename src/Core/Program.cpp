#include "Program.h"
#include <iostream>

#include "ConsoleColours.h"
#include "Services.h"

void Program::Run()
{
	std::cout << "Starting program\n";

	//ColouredConsole::print(ConsoleColour::GREEN, "Starting ");
	//ColouredConsole::print(ConsoleColour::CYAN, "Program",true);

	std::cout << ConsoleColours::getColourCode(AnsiColours::GREEN) << "Starting "
		<< ConsoleColours::getColourCode(AnsiColours::CYAN)<< "program\n";

	Services::initialise();
	auto windowManager = dynamic_cast<WindowManager*>(Services::getService(ServiceType::WindowManager));
	//auto windowManager2 = dynamic_cast<WindowManager*>(Services::getService(ServiceType::WindowManager));
	auto commandConsole = dynamic_cast<CommandConsole*>(Services::getService(ServiceType::CommandConsole));
	auto objectManager = dynamic_cast<ObjectManager*>(Services::getService(ServiceType::ObjectManager));

	windowManager->debugMode = true;

	windowManager->countWindows();
	windowManager->createWindow("Window 1");
	windowManager->createWindow("Window 2");
	windowManager->countWindows();

	windowManager->debugMode = false;
	while (windowManager->countWindows() > 0){
	// program should get call console to get commands
		bool commandLinePause = windowManager->updateWindows();
		if (commandLinePause) {
			commandConsole->parseCommand();
		}
	}
}

Program::Program()
{
}

Program::~Program()
{
}
