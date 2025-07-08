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
	auto commandConsole = dynamic_cast<CommandConsole*>(Services::getService(ServiceType::CommandConsole));

	windowManager->debugMode = true;

	windowManager->countWindows();
	windowManager->createWindow("Window 1");
	windowManager->createWindow("Window 2");
	windowManager->countWindows();

	while (true){
	// program should get call console to get commands
	}
}

Program::Program()
{
}

Program::~Program()
{
}
