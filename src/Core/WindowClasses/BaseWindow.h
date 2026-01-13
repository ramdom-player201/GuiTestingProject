#pragma once

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
public:
	WindowReturnData Update();

	size_t getId() const { return windowId; }

	BaseWindow(size_t id);	// instantiate a window
	~BaseWindow();			// cleanup window by deleting glfwWindow
};