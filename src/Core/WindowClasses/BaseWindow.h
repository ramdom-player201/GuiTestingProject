#pragma once

//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
//#include <string>
//#include <vector>

struct WindowReturnData {
	bool WindowClosed = false;
	bool UserCommandBreak = false;
	bool FocusChanged = false;
	bool WindowInFocus = false;
};

#include "../Services/VulkanHandler.h"

class BaseWindow {
private:
	VulkanHandler& vulkanHandler; // reference to vulkan instance
protected:
	size_t windowId{ 0 };
	GLFWwindow* window{ nullptr };
	VkSurfaceKHR surface{ VK_NULL_HANDLE };
	SwapChainData swapChainData;
public:
	WindowReturnData Update();

	size_t getId() const { return windowId; }

	// safety locks to prevent move/copy/assignment
	BaseWindow() = delete;
	BaseWindow(const BaseWindow&) = delete;
	BaseWindow& operator=(const BaseWindow&) = delete;
	BaseWindow(BaseWindow&&) = delete;
	BaseWindow& operator=(BaseWindow&&) = delete;

	BaseWindow(size_t id, VulkanHandler& vk);	// instantiate a window
	~BaseWindow();			// cleanup window by deleting glfwWindow

	// ClassName
	static constexpr std::string_view className{ "BaseWindow" };
};