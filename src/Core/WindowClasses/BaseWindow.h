#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

struct WindowReturnData {
	bool WindowClosed = false;
	bool UserCommandBreak = false;
	bool FocusChanged = false;
	bool WindowInFocus = false;
};

class BaseWindow {
private:
	BaseWindow() {}; // Cannot contruct a BaseWindow without an id.
protected:
	size_t windowId{ 0 };
	GLFWwindow* window{ nullptr };
	VkSurfaceKHR surface{ VK_NULL_HANDLE };
public:
	WindowReturnData Update();

	size_t getId() const { return windowId; }

	BaseWindow(size_t id);	// instantiate a window
	~BaseWindow();			// cleanup window by deleting glfwWindow

	// ClassName
	static constexpr std::string_view className{ "BaseWindow" };
};