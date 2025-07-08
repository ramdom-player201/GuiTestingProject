#include "WindowManager.h"
#include "../ConsoleColours.h"

void WindowManager::initialise() {
	// may be removed, not currently used
}

void WindowManager::shutdown() {
	// may be removed, may be replaced with destructor or something? depends on whether IService can support that.
}

bool WindowManager::createWindow(const std::string& title) {
	// creates a new Window and adds it to vector
	// intialise the new Window and supply it with an id
	if (debugMode) {
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window Manager > " <<
			ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "CreateWindow() :: " <<
			ConsoleColours::getColourCode(AnsiColours::RED) << "Title = " <<
			ConsoleColours::getColourCode(AnsiColours::YELLOW) << title <<
			ConsoleColours::getColourCode(AnsiColours::RED) << " Id = " <<
			ConsoleColours::getColourCode(AnsiColours::YELLOW) << currentId <<
			"\n";
	}
	std::unique_ptr<Window> window = std::make_unique<Window>(currentId++, title);
	windows.push_back(std::move(window));

	return true;
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

std::vector<Window*> WindowManager::getWindows()
{
	if (debugMode) {
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window Manager > " <<
			ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "GetWindows() :: " <<
			ConsoleColours::getColourCode(AnsiColours::RED) << "Cout = " <<
			ConsoleColours::getColourCode(AnsiColours::YELLOW) << windows.size() <<
			"\n";
	}

	// create a copy of the windows vector
	std::vector<Window*> result;
	result.reserve(windows.size());

	// copy window pointers
	for (const auto& win : windows) {
		result.push_back(win.get());
	}

	return result;
}

std::unique_ptr<Window> WindowManager::getWindowById(int id)
{
	if (debugMode) {
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window Manager > " <<
			ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "GetWindowByID() :: " <<
			ConsoleColours::getColourCode(AnsiColours::RED) << "NOT IMPLEMENTED " <<
			"\n";
	}

	return std::unique_ptr<Window>();
}

bool WindowManager::updateWindows()
{
	bool debugPause = false;
	// update all windows
	for (auto& window : windows) {
		bool winPause = window->Update();
		if (winPause) {
			debugPause = true;
		}
	}
	if (debugPause) {
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window Manager > " <<
			ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "updateWindows() :: " <<
			ConsoleColours::getColourCode(AnsiColours::GREEN) << "USER CMD INTERUPT\n";
	}
	return debugPause;
}
