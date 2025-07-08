#pragma once

#include <SFML/Graphics.hpp>
#include <string>

class Window {
private:
	size_t windowId = 0;
	std::string windowTitle = "Untitled Window";
	sf::RenderWindow window = sf::RenderWindow(sf::VideoMode({ 200u, 200u }), "Untitled Window");;
public:
	bool Update();

	Window(size_t id,std::string const& title);
};