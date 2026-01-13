#pragma once

#include <string>
#include <iostream>
#include <cstdint>

//class ColouredConsole;

enum class AnsiColours :uint8_t {
	DEFAULT,
	// Reds
	RED,
	RED_BRIGHT,
	RED_DARK,
	// Oranges
	ORANGE,
	ORANGE_BRIGHT,
	ORANGE_DARK,
	// Yellows
	YELLOW,
	YELLOW_BRIGHT,
	YELLOW_DARK,
	// Greens
	GREEN,
	GREEN_BRIGHT,
	GREEN_DARK,
	// Cyans
	CYAN,
	CYAN_BRIGHT,
	CYAN_DARK,
	// Blues
	BLUE,
	BLUE_BRIGHT,
	BLUE_DARK,
	// Purples
	PURPLE,
	PURPLE_BRIGHT,
	PURPLE_DARK,
	// Magentas
	MAGENTA,
	MAGENTA_BRIGHT,
	MAGENTA_DARK,
	// Pinks
	PINK,
	PINK_BRIGHT,
	PINK_DARK,
	// Greys
	WHITE,
	GREY_BRIGHT,
	GREY_MEDIUM_BRIGHT,
	GREY_MEDIUM_DARK,
	GREY_DARK,
	BLACK,
	// Special
	FLASH,
	ITALIC,
	UNDERLINE,
	UNDERLINE_DOUBLE,
	STRIKETHROUGH,
};

// "\033[38;2;50;250;50m" 

class ConsoleColours {
public:
	static std::string getColourCode(AnsiColours colour) {
		switch (colour) {
		case AnsiColours::DEFAULT:				return "\033[0m";

		case AnsiColours::RED:					return "\033[38;2;200;0;0m";		// 200,  0,  0
		case AnsiColours::RED_BRIGHT:			return "\033[38;2;250;100;100m";	// 250,100,100
		case AnsiColours::RED_DARK:				return "\033[38;2;150;0;0m";		// 150,  0,  0

		case AnsiColours::ORANGE:				return "\033[38;2;200;100;0m";		// 200,100,  0 
		case AnsiColours::ORANGE_BRIGHT:		return "\033[38;2;250;150;50m";		// 250,150, 50
		case AnsiColours::ORANGE_DARK:			return "\033[38;2;150;50;0m";		// 150, 50,  0 

		case AnsiColours::YELLOW:				return "\033[38;2;200;200;0m";		// 200,200,  0 
		case AnsiColours::YELLOW_BRIGHT:		return "\033[38;2;250;250;0m";		// 250,250, 50
		case AnsiColours::YELLOW_DARK:			return "\033[38;2;150;150;0m";		// 150,150,  0

		case AnsiColours::GREEN:				return "\033[38;2;0;200;0m";		//   0,200,  0
		case AnsiColours::GREEN_BRIGHT:			return "\033[38;2;100;250;100m";	// 100,250,100
		case AnsiColours::GREEN_DARK:			return "\033[38;2;0;150;0m";		//   0,150,  0

		case AnsiColours::CYAN:					return "\033[38;2;0;200;200m";		//   0,200,200
		case AnsiColours::CYAN_BRIGHT:			return "\033[38;2;100;250;250m";	// 150,250,250
		case AnsiColours::CYAN_DARK:			return "\033[38;2;0;150;150m";		//   0,150,150

		case AnsiColours::BLUE:					return "\033[38;2;0;0;200m";		//   0,  0,200
		case AnsiColours::BLUE_BRIGHT:			return "\033[38;2;50;50;250m";		//  50, 50,250
		case AnsiColours::BLUE_DARK:			return "\033[38;2;0;50;150m";		//   0, 50,150

		case AnsiColours::PURPLE:				return "\033[38;2;100;0;200m";		// 100,  0,200
		case AnsiColours::PURPLE_BRIGHT:		return "\033[38;2;150;50;250m";		// 150, 50,250
		case AnsiColours::PURPLE_DARK:			return "\033[38;2;50;0;150m";		//  50,  0,150

		case AnsiColours::MAGENTA:				return "\033[38;2;200;0;200m";		// 200,  0,200
		case AnsiColours::MAGENTA_BRIGHT:		return "\033[38;2;250;50;250m";		// 250, 50,250
		case AnsiColours::MAGENTA_DARK:			return "\033[38;2;150;0;150m";		// 150,  0,150

		case AnsiColours::PINK:					return "\033[38;2;200;100;200m";	// 200,100,200
		case AnsiColours::PINK_BRIGHT:			return "\033[38;2;250;150;250m";	// 250,150,250
		case AnsiColours::PINK_DARK:			return "\033[38;2;150;50;150m";		// 150, 50,150

		case AnsiColours::WHITE:				return "\033[38;2;250;250;250m";	// 250,250,250
		case AnsiColours::GREY_BRIGHT:			return "\033[38;2;200;200;200m";	// 200,200,200
		case AnsiColours::GREY_MEDIUM_BRIGHT:	return "\033[38;2;150;150;150m";	// 150,150,150
		case AnsiColours::GREY_MEDIUM_DARK:		return "\033[38;2;100;100;100m";	// 100,100,100
		case AnsiColours::GREY_DARK:			return "\033[38;2;50;50;50m";		//  50, 50, 50
		case AnsiColours::BLACK:				return "\033[38;2;0;0;0m";			//   0,  0,  0

		case AnsiColours::FLASH:				return "\33[5m";					// Flash
		case AnsiColours::ITALIC:				return "\33[3m";					//
		case AnsiColours::UNDERLINE:			return "\33[4m";					//
		case AnsiColours::UNDERLINE_DOUBLE:		return "\33[21m";					//
		case AnsiColours::STRIKETHROUGH:		return "\33[9m";					//

		default: {
			std::cout << "Invalid colour\n";
			return "";
		}
		}
	}
};

//#pragma once
//
//#include <string>
//#include <iostream>
//
////class ColouredConsole;
//
//enum class ConsoleColour {
//	DEFAULT,
//	RED,
//	ORANGE,
//	YELLOW,
//	GREEN,
//	CYAN,
//	BLUE,
//	PURPLE,
//	MAGENTA
//};
//
//class ColouredConsole {
//private:
//	static std::string getColourCode(ConsoleColour colour) {
//		switch (colour) {
//		case ConsoleColour::DEFAULT: return "\033[0m";
//		case ConsoleColour::RED: return "\033[31m";
//			//case ConsoleColour::ORANGE: return "";
//		case ConsoleColour::YELLOW: return "\033[33m";
//		case ConsoleColour::GREEN: return "\033[32m";
//		case ConsoleColour::CYAN: return  "\033[36m";
//		case ConsoleColour::BLUE: return "\033[34m";
//			//case ConsoleColour::PURPLE: return "";
//		case ConsoleColour::MAGENTA: return "\033[35m";
//		default: {
//			std::cout << "Invalid colour\n";
//			return "";
//		}
//		}
//		}
//
//public:
//	static void print(ConsoleColour colour, const std::string & text, bool newline = false) {
//#ifdef _WIN32
//		// Enable ANSI escape codes on Windows
//		//system("");
//#endif
//		std::cout << getColourCode(colour) << text;
//		if (newline) {
//			std::cout << getColourCode(ConsoleColour::DEFAULT) << "\n";
//		}
//		else {
//			std::cout << getColourCode(ConsoleColour::DEFAULT);
//		}
//	};
//	};