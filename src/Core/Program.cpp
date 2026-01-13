#include "Program.h"
#include <iostream>

#include "ConsoleColours.h"
#include "Services.h"

void Program::Run()
{
	std::cout << ConsoleColours::getColourCode(AnsiColours::GREEN) << "Starting "
		<< ConsoleColours::getColourCode(AnsiColours::CYAN) << "program\n";

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
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Program > "
			<< ConsoleColours::getColourCode(AnsiColours::YELLOW) << "[Constructor] :: "
			<< ConsoleColours::getColourCode(AnsiColours::CYAN) << "GLFW initialised successfully\n";
	}

	std::cout << ConsoleColours::getColourCode(AnsiColours::RED_BRIGHT)			/**/ << "Red Bright\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::RED)				/**/ << "Red\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::RED_DARK)			/**/ << "Red Dark\n";

	std::cout << ConsoleColours::getColourCode(AnsiColours::ORANGE_BRIGHT)		/**/ << "Orange Bright\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::ORANGE)				/**/ << "Orange\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::ORANGE_DARK)		/**/ << "Orange Dark\n";

	std::cout << ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT)		/**/ << "Yellow Bright\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::YELLOW)				/**/ << "Yellow\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::YELLOW_DARK)		/**/ << "Yellow Dark\n";

	std::cout << ConsoleColours::getColourCode(AnsiColours::GREEN_BRIGHT)		/**/ << "Green Bright\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::GREEN)				/**/ << "Green\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::GREEN_DARK)			/**/ << "Green Dark\n";

	std::cout << ConsoleColours::getColourCode(AnsiColours::CYAN_BRIGHT)		/**/ << "Cyan Bright\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::CYAN)				/**/ << "Cyan\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::CYAN_DARK)			/**/ << "Cyan Dark\n";

	std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE_BRIGHT)		/**/ << "Blue Bright\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE)				/**/ << "Blue\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE_DARK)			/**/ << "Blue Dark\n";

	std::cout << ConsoleColours::getColourCode(AnsiColours::PURPLE_BRIGHT)		/**/ << "Purple Bright\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::PURPLE)				/**/ << "Purple\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::PURPLE_DARK)		/**/ << "Purple Dark\n";

	std::cout << ConsoleColours::getColourCode(AnsiColours::MAGENTA_BRIGHT)		/**/ << "Magenta Bright\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::MAGENTA)			/**/ << "Magenta\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::MAGENTA_DARK)		/**/ << "Magenta Dark\n";

	std::cout << ConsoleColours::getColourCode(AnsiColours::PINK_BRIGHT)		/**/ << "Pink Bright\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::PINK)				/**/ << "Pink\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::PINK_DARK)			/**/ << "Pink Dark\n";

	std::cout << ConsoleColours::getColourCode(AnsiColours::WHITE)				/**/ << "White\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::GREY_BRIGHT)		/**/ << "Grey Bright\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT)	/**/ << "Grey Medium Bright\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_DARK)	/**/ << "Grey Medium Dark\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::GREY_DARK)			/**/ << "Grey Dark\n";
	std::cout << ConsoleColours::getColourCode(AnsiColours::BLACK)				/**/ << "Black\n";

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
	std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Program > "
		<< ConsoleColours::getColourCode(AnsiColours::YELLOW) << "[Destructor] :: "
		<< ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "Program closing\n";
	//glfwTerminate(); // Note: in the event of multiple program creations,
					   //       ensure program keeps track of how many exist so this cannot be deinitialised mid-execution
}
