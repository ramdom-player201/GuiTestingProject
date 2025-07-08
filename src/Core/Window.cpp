#include "Window.h"

bool Window::Update()
{
	// process inputs for window and redraw if necassary
	if (window.isOpen()) {
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
				return true;
			}
		}
	}
	return false;
}

Window::Window(size_t id, std::string const& title)
{
	windowId = id;
	windowTitle = title;
}
