#pragma once

#include "IService.h"
#include <string>
#include <memory>
#include <vector>

#include "../Window.h"

class WindowManager : public IService {
private:
	std::vector<std::unique_ptr<Window>> windows;
	size_t currentId = 1;
public:
	//void initialise() override;
	//void shutdown() override;

	// debug
	bool debugMode = false;

	// service-specific functionality
	bool createWindow(const std::string& title); // creates a new window and adds it to list
	size_t countWindows(); // returns number of windows in list
	std::vector<Window*> getWindows(); // returns a copy of the window list
	std::unique_ptr<Window> getWindowById(int id); // returns a copy of a pointer to the window, based on its position in the vector
	void closeWindow(Window* win); // closes the referenced window

	bool updateWindows(); // loop through windows, calling update on each one. Return true if command line break;
};