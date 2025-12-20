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
	// Remember: many Window instances can be created at a time
	// glfwInit() is called in Program.cpp to avoid multiple calls

	// old initialisation
	windowId = id;
	windowTitle = title;
	//window.setTitle(windowTitle);

	// Set GLFW window properties
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // sets version {3}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // sets sub-version {3.3}
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // uses modern API profile?
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);	   // window resizing is allowed

	// Set glfwContext
	glfwMakeContextCurrent(glfwWindow); // This must be called prior to any window update too

	// Load GLAD once after first window creation
	if (!gladInitialised) {
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {

		}
		else {
			gladInitialised = true;
		}
	}

	// GLFW window initialisation
	glfwWindow = glfwCreateWindow(400, 200, title.c_str(), nullptr, nullptr);
	if (glfwWindow) {
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window > "
			<< ConsoleColours::getColourCode(AnsiColours::YELLOW) << "[Constructor] :: "
			<< ConsoleColours::getColourCode(AnsiColours::CYAN) << "GLFW window created\n";
	}
	else {
		// window failed to instantiate
		std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window > "
			<< ConsoleColours::getColourCode(AnsiColours::YELLOW) << "[Constructor] :: "
			<< ConsoleColours::getColourCode(AnsiColours::RED) << "GLFW window failed to create\n";
	}
}

Window::~Window()
{
	std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE) << "Window > "
		<< ConsoleColours::getColourCode(AnsiColours::YELLOW) << "[Destructor] :: "
		<< ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "Destroying window :: "
		<< ConsoleColours::getColourCode(AnsiColours::RED) << "Id = "
		<< ConsoleColours::getColourCode(AnsiColours::YELLOW) << windowId << std::endl;
	if (glfwWindow) {
		glfwDestroyWindow(glfwWindow); // clear window
	}
}