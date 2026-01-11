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
	
	// Set glfwContext
	glfwMakeContextCurrent(glfwWindow); // This must be called prior to any window update
	
	// Load GLAD once after first window creation
	if (!gladInitialised) {
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
	
		}
		else {
			gladInitialised = true;
		}
	}
	
	// Configure viewport
	int width, height;
	glfwGetFramebufferSize(glfwWindow, &width, &height);
	glViewport(0, 0, width, height);
	
	///////////////////////////
	//// TEST RENDERING TEMP //
	///////////////////////////
	//
	//const char* vertexShaderSource = R"(
	//	#version 330 core
	//	layout (location = 0) in vec2 aPos;
	//	
	//	void main()
	//	{
	//	    gl_Position = vec4(aPos, 0.0, 1.0);
	//	}
	//	)";
	//
	//const char* fragmentShaderSource = R"(
	//	#version 330 core
	//	out vec4 FragColor;
	//	
	//	void main()
	//	{
	//	    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // red
	//	}
	//	)";
	//
	//// Vertex shader
	//unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
	//glCompileShader(vertexShader);
	//
	//// Fragment shader
	//unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
	//glCompileShader(fragmentShader);
	//
	//// Shader program
	//unsigned int shaderProgram = glCreateProgram();
	//glAttachShader(shaderProgram, vertexShader);
	//glAttachShader(shaderProgram, fragmentShader);
	//glLinkProgram(shaderProgram);
	//
	//// Cleanup shaders (linked into program now)
	//glDeleteShader(vertexShader);
	//glDeleteShader(fragmentShader);
	//
	//float vertices[] = {
	//	// First triangle
	//	-0.5f, -0.5f,
	//	 0.5f, -0.5f,
	//	 0.5f,  0.5f,
	//
	//	 // Second triangle
	//	  -0.5f, -0.5f,
	//	   0.5f,  0.5f,
	//	  -0.5f,  0.5f
	//};
	//
	//unsigned int VAO, VBO;
	//glGenVertexArrays(1, &VAO);
	//glGenBuffers(1, &VBO);
	//
	//glBindVertexArray(VAO);
	//
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//
	//// Vertex attribute: location = 0, 2 floats per vertex
	//glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);
	//
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
	//
	//glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);
	//
	//glUseProgram(shaderProgram);
	//glBindVertexArray(VAO);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	//
	//glfwSwapBuffers(glfwWindow);


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