#pragma once

#include "IService.h"
#include <string>

class CommandConsole : public IService {
private:
public:
	static void parseCommand(std::string cmd);
	static void parseCommand();
};