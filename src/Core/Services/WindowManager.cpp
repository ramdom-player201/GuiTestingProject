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

std::vector<std::unique_ptr<Window>> WindowManager::getWindows()
{
	return std::vector<std::unique_ptr<Window>>();
}

std::unique_ptr<Window> WindowManager::getWindowById(int id)
{
	return std::unique_ptr<Window>();
}

bool WindowManager::updateWindows()
{
	return false;
}
