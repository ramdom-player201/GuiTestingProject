#include "WindowManager.h"

#include "../ConsoleColours.h"
#include "LogService.h"
#include <vector>

#include "../WindowClasses/TooltipWindow.h"
#include "../WindowClasses/ContextWindow.h"
#include "../WindowClasses/TiledWindow.h"
#include "../WindowClasses/DedicatedWindow.h"
#include "../WindowClasses/TestWindow.h"

//void WindowManager::initialise() {
//	// may be removed, not currently used
//}
//
//void WindowManager::shutdown() {
//	// may be removed, may be replaced with destructor or something? depends on whether IService can support that.
//}

size_t WindowManager::createWindow(const std::string& title, WindowTypes type) {
	constexpr std::string_view functionName{ "CreateWindow" };

	// creates a new Window and adds it to map
	// intialise the new Window and supply it with an id
	LogService::Log(LogType::TRACE, className, functionName,
		"New window :: Title = [" + title + "] :: Id = [" + std::to_string(currentId) + "]"
	);


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
	{
		switch (type) {
		case WindowTypes::TooltipWindow: {
			////////////////////////////////////////////////////
			// CREATE TOOLTIP // CREATE TOOLTIP //
			////////////////////////////////////////////////////

			LogService::Log(LogType::TRACE, className, functionName, "Creating new window of type <TooltipWindow>...");

			std::shared_ptr<TooltipWindow> window = std::make_shared<TooltipWindow>(currentId); // create new window
			windows[currentId] = window;

			LogService::Log(LogType::TRACE, className, functionName, "Created window of type <TooltipWindow>");
			break;

		}
		case WindowTypes::ContextWindow: {
			////////////////////////////////////////////////////
			// CREATE CONTEXT // CREATE CONTEXT //
			////////////////////////////////////////////////////

			LogService::Log(LogType::TRACE, className, functionName, "Creating new window of type <ContextWindow>...");

			std::shared_ptr<ContextWindow> window = std::make_shared<ContextWindow>(currentId); // create new window
			windows[currentId] = window;

			LogService::Log(LogType::TRACE, className, functionName, "Created window of type <ContextWindow>");
			break;

		}
		case WindowTypes::TiledWindow: {
			////////////////////////////////////////////////////////////////////////////////////////////////////////
			// CREATE TILED // CREATE TILED //
			////////////////////////////////////////////////////////////////////////////////////////////////////////

			LogService::Log(LogType::TRACE, className, functionName, "Creating new window of type <TiledWindow>...");

			std::shared_ptr<TiledWindow> window = std::make_shared<TiledWindow>(currentId); // create new window
			windows[currentId] = window;

			LogService::Log(LogType::TRACE, className, functionName, "Created window of type <TiledWindow>");
			break;

		}
		case WindowTypes::DedicatedWindow: {
			////////////////////////////////////////////////////////////////////////////////////////////////////////
			// CREATE DEDICATED // CREATE DEDICATED //
			////////////////////////////////////////////////////////////////////////////////////////////////////////

			LogService::Log(LogType::TRACE, className, functionName, "Creating new window of type <DedicatedWindow>...");

			std::shared_ptr<DedicatedWindow> window = std::make_shared<DedicatedWindow>(currentId); // create new window
			windows[currentId] = window;

			LogService::Log(LogType::TRACE, className, functionName, "Created window of type <DedicatedWindow>");
			break;

		}
		case WindowTypes::TestWindow: {
			////////////////////////////////////////////////////////////////////////////////////////////////////////
			// CREATE TEST // CREATE TEST //
			////////////////////////////////////////////////////////////////////////////////////////////////////////

			LogService::Log(LogType::TRACE, className, functionName, "Creating new window of type <TestWindow>...");

			std::shared_ptr<TestWindow> window = std::make_shared<TestWindow>(currentId); // create new window
			windows[currentId] = window;

			LogService::Log(LogType::TRACE, className, functionName, "Created window of type <TestWindow>");
			break;

		}
		default: {
			////////////////////////////////////////////////////////////////////////////////////////////////////////
			// INVALID WINDOW TYPE // INVALID WINDOW TYPE //
			////////////////////////////////////////////////////////////////////////////////////////////////////////
			LogService::Log(LogType::ERROR, className, functionName, "Attempted to create window of invalid type.");
		}
		}
	}

	return currentId++;
}

size_t WindowManager::countWindows()
{
	constexpr std::string_view functionName{ "CountWindows" };

	if (debugMode) {
		LogService::Log(LogType::TRACE, className, functionName, "Current window count: [" + std::to_string(windows.size()) + "]");
	}

	return windows.size();
}

std::shared_ptr<BaseWindow> WindowManager::getWindowById(size_t id)
{
	constexpr std::string_view functionName{ "GetWindowById" };

	// get window from map by id, error if failed
	if (debugMode) {
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window Manager > " <<
			ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "GetWindowByID() :: " <<
			ConsoleColours::getColourCode(AnsiColours::RED) << "NOT IMPLEMENTED " <<
			"\n";
	}
	return std::shared_ptr<BaseWindow>();
}

void WindowManager::closeWindow(size_t id)
{
	constexpr std::string_view functionName{ "CloseWindow" };

	LogService::Log(LogType::TRACE, className, functionName, "Closing window [" + std::to_string(id) + "]");
	//std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Windows Manager > " <<
	//	ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "CloseWIndows() :: " <<
	//	ConsoleColours::getColourCode(AnsiColours::YELLOW) << "Closing Window" <<
	//	ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "\n";;
	windows.erase(id);
}

bool WindowManager::updateWindows()
{
	constexpr std::string_view functionName{ "UpdateWindows" };

	bool debugPause = false;
	// update all windows
	std::vector<size_t> toClose;
	for (auto& windowPair : windows) {
		WindowReturnData winData = windowPair.second->Update();
		if (winData.UserCommandBreak) {
			debugPause = true;
		}
		if (winData.WindowClosed) {
			// delete window from vector and update id
			LogService::Separator();
			LogService::Log(LogType::USER, className, functionName, "WINDOW CLOSE REQUESTED");
			toClose.push_back(windowPair.first); // cannot close window directly in loop
		}
	}
	for (size_t id : toClose) { // wait until after updating to close
		closeWindow(id);
	}
	if (debugPause) {
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window Manager > " <<
			ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "updateWindows() :: " <<
			ConsoleColours::getColourCode(AnsiColours::GREEN) << "USER CMD INTERUPT" <<
			ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "\n";
	}
	return debugPause;
}
