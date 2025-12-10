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
	focusConsole();
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

#ifdef _WIN64
#include <Windows.h>
#endif

// windows only rn, linux support is for when I have a linux device to test with
void CommandConsole::focusConsole()
{
	std::cout << ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "WIP: CommandConsole -> Console may not work correctly on linux, additional testing recommended\n";
#ifdef _WIN64
	HWND consoleHandle = GetConsoleWindow();
	if (consoleHandle) {
		ShowWindow(consoleHandle, SW_RESTORE);
		SetForegroundWindow(consoleHandle);
	}
#else
	std::cout << ConsoleColours::getColourCode(AnsiColours::RED) << "WIP: CommandConsole -> This feature is currently disabled for Mac and Linux\n";
#endif
}
