#include "Program.h"
#include <iostream>

#include "ConsoleColours.h"
#include "Services.h"

void Program::Run()
{
	std::cout << ConsoleColours::getColourCode(AnsiColours::GREEN) << "Starting "
		<< ConsoleColours::getColourCode(AnsiColours::CYAN) << "program\n";
	LogService::Log(
		LogType::TRACE,
		className,
		"Run",
		"Program Start"
	);

	VulkanHandler::Initialise();

	Services::initialise();
	//auto windowManager = dynamic_cast<WindowManager*>(Services::getService(ServiceType::WindowManager));
	//auto windowManager2 = dynamic_cast<WindowManager*>(Services::getService(ServiceType::WindowManager));
	auto commandConsole = dynamic_cast<CommandConsole*>(Services::getService(ServiceType::CommandConsole));
	//auto objectManager = dynamic_cast<ObjectManager*>(Services::getService(ServiceType::ObjectManager));

	WindowManager::debugMode = true;

	WindowManager::countWindows();
	WindowManager::createWindow("Window 1", WindowTypes::TestWindow);
	WindowManager::createWindow("Window 2", WindowTypes::TestWindow);
	WindowManager::countWindows();

	WindowManager::createWindow("Window 33", WindowTypes::TestWindow);

	WindowManager::debugMode = false;
	while (WindowManager::countWindows() > 0) {
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
		LogService::Log(
			LogType::CRITICAL,
			className,
			"Constructor",
			"GLFW failed to initialise"
		);
		std::cerr << "Failed to initialize GLFW" << std::endl;
	}
	else {
		LogService::Log(
			LogType::SUCCESS,
			className,
			"Constructor",
			"GLFW initialised successfully"
		);
	}

	ConsoleColours::PrintTestColours();

	LogService::Log(LogType::CRITICAL,className,"Constructor","Testing LogTypes");
	LogService::Log(LogType::ERROR, className, "Constructor", "Testing LogTypes");
	LogService::Log(LogType::ABNORM, className, "Constructor", "Testing LogTypes");
	LogService::Log(LogType::WIP, className, "Constructor", "Testing LogTypes");
	LogService::Log(LogType::SECURITY, className, "Constructor", "Testing LogTypes");

	LogService::Log(LogType::HIGH, className, "Constructor", "Testing LogTypes");
	LogService::Log(LogType::MED, className, "Constructor", "Testing LogTypes");
	LogService::Log(LogType::LOW, className, "Constructor", "Testing LogTypes");

	LogService::Log(LogType::SUCCESS, className, "Constructor", "Testing LogTypes");

	LogService::Log(LogType::TRACE, className, "Constructor", "Testing LogTypes");
	LogService::Log(LogType::SPAM, className, "Constructor", "Testing LogTypes");
	LogService::Log(LogType::CATCH, className, "Constructor", "Testing LogTypes");

	//std::cout <<
	//	"\033[0m" << "Test ansi colours" << "\033[0m" <<
	//	"\033[1m" << " 1m " << "\033[0m" <<
	//	"\033[2m" << " 2m " << "\033[0m" <<
	//	"\033[3m" << " 3m " << "\033[0m" <<
	//	"\033[4m" << " 4m " << "\033[0m" <<
	//	"\033[5m" << " 5m " << 
	//	"\033[7m" << " 7m " << "\033[0m" <<
	//	"\033[8m" << " 8m " << "\033[0m" <<
	//	"\033[9m" << " 9m " << "\033[0m" << // strike through
	//	"\033[21m" << " 21m " << "\033[0m" << // double lined
	//	"\033[24m" << " 24m " << "\033[0m" << // underlined
	//	"\033[5m" << " 5m " <<
	//	std::endl;
}

Program::~Program()
{
	LogService::Log(LogType::TRACE, className, "Destructor", "Program Closing");
	LogService::Log(LogType::WIP, className, "Destructor", "Note: WindowManager needs to clear all windows prior to GLFW Terminate");
	glfwTerminate(); // Note: in the event of multiple program creations,
					   //       ensure program keeps track of how many exist so this cannot be deinitialised mid-execution
					   // ??? window creation or program? program already initialises when created. 
	                   // If program goes out of scope, WindowManager persists as is static.
	LogService::Log(LogType::TRACE, className, "Destructor", "GLFW Terminated");
}
