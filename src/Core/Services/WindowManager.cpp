#include "WindowManager.h"

#include "LogService.h"
#include "../ConsoleColours.h"
#include "../ProgramContexts/Base_AbstractContext.h"

#include <vector>

uint32_t WindowManager::CreateWindow(const std::string& title, Base_AbstractContext* owner) {
	constexpr std::string_view functionName{ "CreateWindow" };

	// creates a new Window and adds it to map
	// intialise the new Window and supply it with an id
	LogService::Log(LogType::TRACE, className, functionName,
		std::format("New window :: Title = [{}{}{}] :: Id = [{}{}{}]",
			ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT), title,
			ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT),
			ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT), currentId,
			ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT)
		)
	);

	constexpr int defaultWidth{ 600 };
	constexpr int defaultHeight{ 400 };

	if (debugMode) {
		std::cout << "WIP: ideally, windowId should never go backwards, add an assert/warning to ensure you are always writing to empty\n";
	}

	// Create window and add to map
	LogService::Log(LogType::TRACE, className, functionName, "Creating new window");
	WindowEntry entry;
	entry.window = std::make_unique<BaseWindow>(currentId, vulkanHandler, defaultWidth, defaultHeight, title);
	entry.owningContext = owner;
	windows[currentId] = std::move(entry);

	return currentId++;
}

BaseWindow* WindowManager::GetWindowById(uint32_t id) const
{
	constexpr std::string_view functionName{ "GetWindowById" };
	
	auto it = windows.find(id);
	if (it == windows.end()) { 
		LogService::Log(LogType::ERROR, className, functionName, "Window not found");
		return nullptr; // Window not found
	} 
	LogService::Log(LogType::TRACE, className, functionName, std::format("Found window for id [{}]", id));
	return it->second.window.get(); // Window found
}

void WindowManager::CloseWindow(uint32_t id)
{
	constexpr std::string_view functionName{ "CloseWindow" };

	LogService::Log(LogType::TRACE, className, functionName,
		std::format("Closing window [{}{}{}]",
			ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT), id,
			ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT)
		)
	);

	auto it = windows.find(id);
	if (it != windows.end()) {
		LogService::Log(LogType::TRACE, className, functionName, "Closed window");
		windows.erase(it);
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
	std::vector<uint32_t> requestedClosures;
	for (auto& [id, entry] : windows) {
		WindowReturnData winData = entry.window->Update();

		if (winData.UserCommandBreak) { debugPause = true; }
		if (winData.WindowCloseRequested) {
			// Mark window for deletion
			LogService::Log(LogType::USER, className, functionName,
				std::format("{}WINDOW CLOSE REQUESTED{}",
					ConsoleColours::getColourCode(AnsiColours::GREEN_BRIGHT),
					ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT)
				)
			);
			// Cannot mutate the window list by immediately destroying windows while still iterating
			requestedClosures.push_back(id); // Defer closure
		}
	}

	// Wait until after all updates before closing
	for (uint32_t id : requestedClosures) {
		auto it{ windows.find(id) }; // find window in vector
		if (it == windows.end()) { continue; } // skip if missing

		bool canClose{ true };
		if (it->second.owningContext) { // query owning context on whether window is allowed to close
			canClose = it->second.owningContext->HandleClosureRequest(id);
			// Note: it is safe to mutate WindowManager state in the post-update closure processing
			// ^ As an example, OnCloseRequested might spawn a confirmation popup window
		}

		if (canClose) {
			CloseWindow(id); // Id is stable, even if window list was mutated
		}
		else { // Default is true, only false if callback vetoes
			LogService::Log(
				LogType::MED, className, functionName, std::format("Closure vetoed by owning context of window [{}]", id)
			);
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

WindowManager::~WindowManager()
{
	// Force close all windows
	windows.clear();
	LogService::Log(LogType::TRACE, className, "Destructor", "WindowManager destroyed");
}