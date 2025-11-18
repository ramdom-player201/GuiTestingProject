#pragma once

//#include "IService.h"
#include <string>
#include <memory>
#include <vector>

#include "../Window.h"

class WindowManager {
private:
	static inline std::vector<std::unique_ptr<Window>> windows;
	static inline size_t currentId{ 1 };
public:
	//void initialise() override;
	//void shutdown() override;

	// debug
	static inline bool debugMode{ false };

	// service-specific functionality
	static bool createWindow(const std::string& title);		// creates a new window and adds it to list
	static size_t countWindows();							// returns number of windows in list
	static std::vector<Window*> getWindows();				// returns a copy of the window list
	static std::unique_ptr<Window> getWindowById(int id);	// returns a copy of a pointer to the window, based on its position in the vector
	static void closeWindow(Window* win);					// closes the referenced window

	static bool updateWindows();							// loop through windows, calling update on each one. Return true if command line break;
};