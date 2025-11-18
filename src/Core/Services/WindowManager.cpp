#include "WindowManager.h"
#include "../ConsoleColours.h"

//void WindowManager::initialise() {
//	// may be removed, not currently used
//}
//
//void WindowManager::shutdown() {
//	// may be removed, may be replaced with destructor or something? depends on whether IService can support that.
//}

size_t WindowManager::createWindow(const std::string& title) {
	// creates a new Window and adds it to map
	// intialise the new Window and supply it with an id
	if (debugMode) {
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window Manager > " <<
			ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "CreateWindow() :: " <<
			ConsoleColours::getColourCode(AnsiColours::RED) << "Title = " <<
			ConsoleColours::getColourCode(AnsiColours::YELLOW) << title <<
			ConsoleColours::getColourCode(AnsiColours::RED) << " Id = " <<
			ConsoleColours::getColourCode(AnsiColours::YELLOW) << currentId <<
			ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "\n";
		std::cout << "WIP: ideally, windowId should never go backwards, add an assert/warning to ensure you are always writing to empty\n";
	}
	std::shared_ptr<Window> window = std::make_shared<Window>(currentId, title);
	windows.insert({ currentId, window });

	return currentId++;
}

size_t WindowManager::countWindows()
{
	if (debugMode) {
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window Manager > " <<
			ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "CountWindows() :: " <<
			ConsoleColours::getColourCode(AnsiColours::RED) << "Count = " <<
			ConsoleColours::getColourCode(AnsiColours::YELLOW) << windows.size() <<
			"\n";
	}
	return windows.size();
}

//std::vector<Window*> WindowManager::getWindows()
//{
//	if (debugMode) {
//		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window Manager > " <<
//			ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "GetWindows() :: " <<
//			ConsoleColours::getColourCode(AnsiColours::RED) << "Cout = " <<
//			ConsoleColours::getColourCode(AnsiColours::YELLOW) << windows.size() <<
//			"\n";
//	}
//
//	// create a copy of the windows vector
//	std::vector<Window*> result;
//	result.reserve(windows.size());
//
//	// copy window pointers
//	for (const auto& win : windows) {
//		result.push_back(win.get());
//	}
//
//	return result;
//}

std::shared_ptr<Window> WindowManager::getWindowById(size_t id)
{
	// get window from map by id, error if failed
	if (debugMode) {
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window Manager > " <<
			ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "GetWindowByID() :: " <<
			ConsoleColours::getColourCode(AnsiColours::RED) << "NOT IMPLEMENTED " <<
			"\n";
	}
	return std::shared_ptr<Window>();
}

void WindowManager::closeWindow(size_t id)
{
	std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Windows Manager" <<
		ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "closeWIndows() ::" <<
		ConsoleColours::getColourCode(AnsiColours::RED) << "COULD NOT CLOSE WINDOW: ERROR WIP" <<
		ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "\n";;
}

bool WindowManager::updateWindows()
{
	bool debugPause = false;
	// update all windows
	for (auto& window : windows) {
		WindowReturnData winData = window.second->Update();
		if (winData.UserCommandBreak) {
			debugPause = true;
		}
		if (winData.WindowClosed) {
			// delete window from vector and update id
			std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window Manager > " <<
				ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "updateWindows() :: " <<
				ConsoleColours::getColourCode(AnsiColours::RED) << "WINDOW CLOSE DETECTED: ERROR WIP" <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "\n";

		}
	}
	if (debugPause) {
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window Manager > " <<
			ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "updateWindows() :: " <<
			ConsoleColours::getColourCode(AnsiColours::GREEN) << "USER CMD INTERUPT" <<
			ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "\n";
	}
	return debugPause;
}
