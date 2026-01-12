//#pragma once
//
//#include "glad/glad.h"
//#include <GLFW/glfw3.h>
//
////#include <SFML/Graphics.hpp>
//#include <string>
//#include "Pages/WinTile.h"
//
//struct WindowReturnData {
//	bool WindowClosed = false;
//	bool UserCommandBreak = false;
//	bool FocusChanged = false;
//	bool WindowInFocus = false;
//};
//
//struct Tiles {
//	WinTile leftTile{ TileType::side };
//	WinTile rightTile{ TileType::side };
//	WinTile centreTile{ TileType::centred };
//	WinTile bottomTile{ TileType::centred };
//};
//
//class Window {
//private:
//	size_t windowId = 0;
//	std::string windowTitle = "Untitled Window";
//
//	// GLFW window
//	GLFWwindow* glfwWindow = nullptr;
//	static bool inline gladInitialised{ false };
//
//	Tiles windowTiles;	// see WindowTilesLayout.png and Pages.txt
//
//	bool windowInFocus{ false };
//
//public:
//	WindowReturnData Update();
//
//	size_t getId() const { return windowId; }
//
//	Window(size_t id, std::string const& title); // instantiate a window, with a title
//	~Window();									 // cleanup window by deleting glfwWindow
//};