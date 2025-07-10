#include "Window.h"

WindowReturnData Window::Update()
{
	WindowReturnData returnData;
	// process inputs for window and redraw if necassary
	if (window.isOpen()) {
		// handle window-specific events
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>()) // window closed, return closed for deletion
			{
				window.close();
				returnData.WindowClosed = true;
				returnData.UserCommandBreak = true; // debug
				
			}
			else if (event->is<sf::Event::Resized>()) { // window resized, force redraw

			}
		}
		// request inputs if primary
		//<>
		// call draw for all UI components
		//<>
	}
	return returnData;
}

Window::Window(size_t id, std::string const& title)
{
	windowId = id;
	windowTitle = title;
}
