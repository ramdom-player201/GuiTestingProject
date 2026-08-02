#include "WindowManager.h"

#include "../ConsoleColours.h"
#include "LogService.h"
#include <vector>

size_t WindowManager::CreateWindow(const std::string& title, WindowTypes type) {
	constexpr std::string_view functionName{ "CreateWindow" };

	// creates a new Window and adds it to map
	// intialise the new Window and supply it with an id
	LogService::Log(LogType::TRACE, className, functionName,
		"New window :: Title = [" +
		ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT) + title +
		ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT) + "] :: Id = [" +
		ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT) + std::to_string(currentId) +
		ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT) + "]"
	);

	constexpr int defaultWidth{ 600 };
	constexpr int defaultHeight{ 400 };

	if (debugMode) {
		//std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window Manager > " <<
		//	ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "CreateWindow() :: " <<
		//	ConsoleColours::getColourCode(AnsiColours::RED) << "Title = " <<
		//	ConsoleColours::getColourCode(AnsiColours::YELLOW) << title <<
		//	ConsoleColours::getColourCode(AnsiColours::RED) << " Id = " <<
		//	ConsoleColours::getColourCode(AnsiColours::YELLOW) << currentId <<
		//	ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "\n";
		std::cout << "WIP: ideally, windowId should never go backwards, add an assert/warning to ensure you are always writing to empty\n";
	}

	// Create window of requested type
	LogService::Log(LogType::TRACE, className, functionName, "Creating new window");
	windows[currentId] = std::make_unique<BaseWindow>(currentId, vulkanHandler, defaultWidth, defaultHeight, title); // create new window
	LogService::Log(LogType::WIP, className, functionName, "WindowType is now legacy, since BaseWindow subclasses are no longer relevant");

	return currentId++;
}

// int temp{ 0 };

size_t WindowManager::CountWindows() const
{
	constexpr std::string_view functionName{ "CountWindows" };

	//if (debugMode) {
	LogService::Log(LogType::SPAM, className, functionName, "Current window count: [" + std::to_string(windows.size()) + "]");
	//}

		//temp++;
		//if (temp > 1000000) {
		//	temp = 0;
		//	LogService::Log(LogType::SPAM, className, functionName, "To Differ");
		//}

	return windows.size();
}

BaseWindow* WindowManager::GetWindowById(size_t id) const
{
	constexpr std::string_view functionName{ "GetWindowById" };

	// get window from map by id, error if failed
	//if (debugMode) {
	//	std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window Manager > " <<
	//		ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "GetWindowByID() :: " <<
	//		ConsoleColours::getColourCode(AnsiColours::RED) << "NOT IMPLEMENTED " <<
	//		"\n";
	//}
	LogService::Log(LogType::HIGH, className, functionName, "This class should have mutexes implemented before use in multi-threading");
	auto it = windows.find(id);
	if (it != windows.end()) {
		LogService::Log(LogType::TRACE, className, functionName, "Found window for id [" + std::to_string(id) + "]");
		return it->second.get();
	}
	LogService::Log(LogType::ERROR, className, functionName, "No window found for id [" + std::to_string(id) + "]");
	return nullptr;
}

void WindowManager::CloseWindow(size_t id)
{
	constexpr std::string_view functionName{ "CloseWindow" };

	LogService::Log(LogType::TRACE, className, functionName,
		"Closing window [" +
		ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT) +
		std::to_string(id) +
		ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT) + "]"
	);
	//std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Windows Manager > " <<
	//	ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "CloseWIndows() :: " <<
	//	ConsoleColours::getColourCode(AnsiColours::YELLOW) << "Closing Window" <<
	//	ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "\n";;
	auto it = windows.find(id);
	if (it != windows.end()) {
		LogService::Log(LogType::TRACE, className, functionName, "Closed window");
		windows.erase(id);
	}
	else {
		LogService::Log(LogType::ERROR, className, functionName, "Failed to close window because invalid id");
	}
}

bool WindowManager::UpdateWindows()
{
	constexpr std::string_view functionName{ "UpdateWindows" };
	bool debugPause = false;

	glfwPollEvents();

	// Update all windows
	std::vector<size_t> toClose;
	for (auto& windowPair : windows) {
		WindowReturnData winData = windowPair.second->Update();

		if (winData.UserCommandBreak) { debugPause = true; }
		if (winData.WindowClosed) {
			// Delete window from vector and update id
			LogService::Separator();
			LogService::Log(LogType::USER, className, functionName,
				ConsoleColours::getColourCode(AnsiColours::GREEN_BRIGHT) +
				"WINDOW CLOSE REQUESTED" +
				ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT)
			);
			toClose.push_back(windowPair.first); // Cannot close window directly in loop
		}
	}
	for (size_t id : toClose) { // Wait until after updating to close
		CloseWindow(id);
	}
	if (debugPause) {
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window Manager > " <<
			ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "updateWindows() :: " <<
			ConsoleColours::getColourCode(AnsiColours::GREEN) << "USER CMD INTERUPT" <<
			ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "\n";
	}
	return debugPause;
}

WindowManager::~WindowManager()
{
	LogService::Log(LogType::TRACE, className, "Destructor", "WindowManager destroyed");
	// Force close all windows safely
	for (auto& pair : windows) {
		pair.second.reset(); // triggers BaseWindow destructor
	}
	windows.clear();
}