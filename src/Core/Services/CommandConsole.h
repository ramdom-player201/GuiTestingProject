#pragma once

#include "IService.h"
#include <string>

class CommandConsole : public IService {
private:
public:
	static void parseCommand(std::string cmd);	// process a command
	static void parseCommand();					// get input and call parse on input (also calls focusConsole)
	static void focusConsole();					// bring console to forefront
};