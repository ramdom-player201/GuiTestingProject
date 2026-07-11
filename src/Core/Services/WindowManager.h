#pragma once

//#include "IService.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "../Window.h"
#include "../WindowClasses/BaseWindow.h"

// Valid Window Types
enum class WindowTypes : uint8_t {
	TooltipWindow,
	ContextWindow,
	TiledWindow,
	DedicatedWindow,
	TestWindow,
};

class WindowManager {
private:
	//static inline std::vector<std::unique_ptr<Window>> windows;
	std::unordered_map<size_t, std::unique_ptr<BaseWindow>> windows;	// list of windows with unique numeric id that does not shift
	size_t currentId{ 0 };												// initial id
	VulkanHandler& vulkanHandler;
public:
	//void initialise() override;
	//void shutdown() override;

	// debug
	bool debugMode{ false };

	// service-specific functionality
	size_t CreateWindow(const std::string& title, WindowTypes type);	// creates a new window and adds it to list
	size_t CountWindows() const;										// returns number of windows in list
	BaseWindow* GetWindowById(size_t id) const;							// returns a copy of a pointer to the window, based on its position in the vector
	void CloseWindow(size_t id);										// closes the referenced window

	bool UpdateWindows();												// loop through windows, calling update on each one. Return true if command line break;

	// safety locks, plus destructor
	WindowManager(VulkanHandler& vk) :vulkanHandler(vk) {};
	WindowManager() = delete;
	WindowManager(const WindowManager&) = delete;
	WindowManager& operator=(const WindowManager&) = delete;
	WindowManager(WindowManager&&) = delete;
	WindowManager& operator=(WindowManager&&) = delete;
	~WindowManager();

	// ClassName
	static constexpr std::string_view className{ "WindowManager" };
};