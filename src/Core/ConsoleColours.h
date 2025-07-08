#pragma once

#include <string>
#include <iostream>

//class ColouredConsole;

enum class AnsiColours {
	DEFAULT,
	RED,
	ORANGE,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	PURPLE,
	MAGENTA
};

class ConsoleColours {
public:
	static std::string getColourCode(AnsiColours colour) {
		switch (colour) {
		case AnsiColours::DEFAULT: return "\033[0m";
		case AnsiColours::RED: return "\033[31m";
			//case ConsoleColour::ORANGE: return "";
		case AnsiColours::YELLOW: return "\033[33m";
		case AnsiColours::GREEN: return "\033[32m";
		case AnsiColours::CYAN: return  "\033[36m";
		case AnsiColours::BLUE: return "\033[34m";
			//case ConsoleColour::PURPLE: return "";
		case AnsiColours::MAGENTA: return "\033[35m";
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