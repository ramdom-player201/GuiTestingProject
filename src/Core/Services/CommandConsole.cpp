#include "CommandConsole.h"

#include "iostream"
#include "limits"

#include "../ConsoleColours.h"

// process given command
void CommandConsole::parseCommand(std::string cmd)
{
	bool commandProcessed{ false }; // command is failed by default
	std::cout << ConsoleColours::getColourCode(AnsiColours::RED) << "CommandConsole not yet implemented\n";

}

// take input and call parse on it
void CommandConsole::parseCommand()
{
	std::cout << ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "\nAwaiting User Input: type help for more info.\n" << "U->";
	std::string input;
	std::cin >> input;
	if (std::cin.fail()) {
		std::cout << ConsoleColours::getColourCode(AnsiColours::RED) << "[CmdConsole|ERROR]: Unknown input error\n";
	}
	std::cin.clear();
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cout << "[User]: " << input << "\n";
	parseCommand(input);
}
