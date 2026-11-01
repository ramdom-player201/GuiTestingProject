#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "../WindowClasses/BaseWindow.h"

class Base_AbstractContext; // Forward declaration

class WindowManager {
private:
	struct WindowEntry {
		std::unique_ptr<BaseWindow> window;
		Base_AbstractContext* owningContext{ nullptr }; // Context backend that owns this frontend window
	};

	std::unordered_map<uint32_t, WindowEntry> windows;	// list of windows with unique numeric id that does not shift
	uint32_t currentId{ 0 };							// initial id
	VulkanHandler& vulkanHandler;

public:
	// debug
	bool debugMode{ false };

	// service-specific functionality
	uint32_t CreateWindow(
		const std::string& title, Base_AbstractContext* owner = nullptr // creates a new window and adds it to list
	);					
	size_t CountWindows() const { return windows.size(); }				// returns number of windows in list
	BaseWindow* GetWindowById(uint32_t id) const;						// returns a pointer to the window, or nullptr if not found
	void CloseWindow(uint32_t id);										// closes the referenced window (bypasses owner callback)

	bool UpdateWindows();	// loop through windows, calling update on each one. Return true if command line break;
	// Note: might deprecate commandline break

	// safety locks, plus destructor
	WindowManager(VulkanHandler& vk) : vulkanHandler(vk) { LogService::Log(LogType::TRACE, className, "Constructor", "Created Window Manager"); };
	WindowManager() = delete;
	WindowManager(const WindowManager&) = delete;
	WindowManager& operator=(const WindowManager&) = delete;
	WindowManager(WindowManager&&) = delete;
	WindowManager& operator=(WindowManager&&) = delete;
	~WindowManager();

	// ClassName
	static constexpr std::string_view className{ "WindowManager" };
};