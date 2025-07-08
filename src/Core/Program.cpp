#include "Program.h"
#include <iostream>

#include "ColouredConsole.h"

void Program::Run()
{
	std::cout << "Starting program\n";

	//ColouredConsole::print(ConsoleColour::GREEN, "Starting ");
	//ColouredConsole::print(ConsoleColour::CYAN, "Program",true);

	std::cout << ConsoleColours::getColourCode(AnsiColours::GREEN) << "Starting "
		<< ConsoleColours::getColourCode(AnsiColours::CYAN)<< "program\n";

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
