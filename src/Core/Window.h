#pragma once

#include <SFML/Graphics.hpp>
#include <string>

struct WindowReturnData {
	bool WindowClosed = false;
	bool UserCommandBreak = false;
	bool WindowInFocus = false;
};

class Window {
private:
	size_t windowId = 0;
	std::string windowTitle = "Untitled Window";
	sf::RenderWindow window = sf::RenderWindow(sf::VideoMode({ 200u, 200u }), "Untitled Window");;
	//const IService* windowManager = dynamic_cast<WindowManager*>(Services::getService(ServiceType::WindowManager));
public:
	WindowReturnData Update();

	Window(size_t id,std::string const& title);
};