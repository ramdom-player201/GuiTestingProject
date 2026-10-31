#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "../WindowClasses/BaseWindow.h"

class WindowManager {
private:
	//static inline std::vector<std::unique_ptr<Window>> windows;
	std::unordered_map<uint32_t, std::unique_ptr<BaseWindow>> windows;	// list of windows with unique numeric id that does not shift
	uint32_t currentId{ 0 };											// initial id
	VulkanHandler& vulkanHandler;
public:

	// debug
	bool debugMode{ false };

	// service-specific functionality
	uint32_t CreateWindow(const std::string& title);					// creates a new window and adds it to list
	uint32_t CountWindows() const;										// returns number of windows in list
	BaseWindow* GetWindowById(uint32_t id) const;						// returns a copy of a pointer to the window, based on its position in the vector
	void CloseWindow(uint32_t id);										// closes the referenced window

	bool UpdateWindows();												// loop through windows, calling update on each one. Return true if command line break;

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