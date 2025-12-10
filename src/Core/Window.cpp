#include "Window.h"

#include "ConsoleColours.h"

WindowReturnData Window::Update()
{
	WindowReturnData returnData;

	// ---------------------------------------------------------------------------- //
	// SFML implementation
	// ---------------------------------------------------------------------------- //

	// process inputs for window and redraw if necassary
	/*if (window.isOpen()) {
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
			else if (event->is<sf::Event::FocusGained>()) {
				windowInFocus = true;
				returnData.FocusChanged = true;
			}
			else if (event->is<sf::Event::FocusLost>()) {
				windowInFocus = false;
				returnData.FocusChanged = true;
			}

		}
		// request inputs if primary
		//<>
		// call draw for all UI components
		//<>
	}*/

	// ---------------------------------------------------------------------------- //
	// GLFW implementation
	// ---------------------------------------------------------------------------- //

	glfwPollEvents(); // process events

	// Check if window should close
	if (glfwWindowShouldClose(glfwWindow)) {
		returnData.WindowClosed = true;
		returnData.UserCommandBreak = true;
	}


	return returnData;
}

Window::Window(size_t id, std::string const& title)
{
	// old initialisation
	windowId = id;
	windowTitle = title;
	//window.setTitle(windowTitle);

	// GLFW window initialisation
	glfwWindow = glfwCreateWindow(400, 200, title.c_str(), nullptr, nullptr);
	if (glfwWindow) {
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window > "
			<< ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "Initialise() :: "
			<< ConsoleColours::getColourCode(AnsiColours::CYAN) << "GLFW window created\n";
	}
	else {
		// window failed to instantiate
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window > "
			<< ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "Initialise() :: "
			<< ConsoleColours::getColourCode(AnsiColours::RED) << "GLFW window failed to create\n";
	}
}

Window::~Window()
{
	std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window > "
		<< ConsoleColours::getColourCode(AnsiColours::MAGENTA) << "Destructor() :: "
		<< ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "Destroying window :: "
		<< ConsoleColours::getColourCode(AnsiColours::RED) << "Id = "
		<< ConsoleColours::getColourCode(AnsiColours::YELLOW) << windowId << std::endl;
	if (glfwWindow) {
		glfwDestroyWindow(glfwWindow); // clear window
	}
}