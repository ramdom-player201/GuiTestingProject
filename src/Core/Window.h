#pragma once

#include <string>

class Window {
private:
	size_t windowId = 0;
	std::string windowTitle = "Untitled Window";
public:
	Window(size_t id,std::string const& title);
};