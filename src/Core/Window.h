#pragma once

#include <GLFW/glfw3.h>

#include <SFML/Graphics.hpp>
#include <string>
#include "Pages/WinTile.h"

struct WindowReturnData {
	bool WindowClosed = false;
	bool UserCommandBreak = false;
	bool FocusChanged = false;
	bool WindowInFocus = false;
};

struct Tiles {
	WinTile leftTile{TileType::side};
	WinTile rightTile{ TileType::side };
	WinTile centreTile{ TileType::centred };
	WinTile bottomTile{ TileType::centred };
};

class Window {
private:
	size_t windowId = 0;
	std::string windowTitle = "Untitled Window";
	sf::RenderWindow window = sf::RenderWindow(sf::VideoMode({ 400u, 200u }), "Untitled Window");;
	//const IService* windowManager = dynamic_cast<WindowManager*>(Services::getService(ServiceType::WindowManager));

	Tiles windowTiles;	// see WindowTilesLayout.png and Pages.txt

	bool windowInFocus{ false };

public:
	WindowReturnData Update();

	Window(size_t id,std::string const& title);
};