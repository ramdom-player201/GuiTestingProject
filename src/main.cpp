//#include <SFML/Graphics.hpp>

#include "Core/Program.h"
#include "Core/Services/LogService.h"
#include "Core/Services/FileManager.h"
#include <iostream>

#include "Core/GuiLoading/Lexer.h" // Temporary for testing

int main()
{
	std::cout << "Log size: " << LogService::GetCurrentDataUsage() << std::endl;

	// quick tests
	//LogService::Log(LogType::TEST, "main", "main", "Running temporary file manager tests.");
	//FileManager::ValidatePath("Place");
	//FileManager::ValidatePath("Place/A/B");
	//FileManager::ValidatePath("Place/A/B/../D");
	//FileManager::ValidatePath("Place/../../../../A");
	//FileManager::ValidatePath("HyperSphere/TextFile.txt");
	//FileManager::ValidatePath("Place/../../../../Test.txt");
	//FileManager::ValidatePath("HyperCube/Cube.cube.txt");
	//FileManager::ValidatePath("HyperSphere/Cube.txt/A");

	// main code
	LogService::Initialise();

//	std::string source = R"(
//@ Lexer test sample
//@ Exercises every token type at least once
//
//UiFr : root
//	siz = {sx: 1 sy: 0.5}
//
//UiSh : panel
//	col = #FF0000FF
//	accent = #F00
//	border = #F00F
//	surface = #FF0000
//	pos.ox = -5
//	pos.oy = +10
//	name = "test"
//	errorState = "bob
//	value = a_b_c_d
//	flag = _
//	theme = <Light : Accent>
//
//Build:
//	root <- panel
//
//Output:
//	root
//        )";

	//Lexer l(source);
	//bool finished{ false };
	//while (!finished) {
	//	Token output = l.NextToken();
	//	std::cout << "Token t = <";
	//	switch (output.type) {
	//	case TokenType::Ident:		std::cout << "Ident"; break;
	//	case TokenType::Number:		std::cout << "Number"; break;
	//	case TokenType::Hex:		std::cout << "Hex"; break;
	//	case TokenType::String:		std::cout << "String"; break;
	//	case TokenType::BraceL:		std::cout << "BraceL"; break;
	//	case TokenType::BraceR:		std::cout << "BraceR"; break;
	//	case TokenType::Colon:		std::cout << "Colon"; break;
	//	case TokenType::Assign:		std::cout << "Assign"; break;
	//	case TokenType::Dot:		std::cout << "Dot"; break;
	//	case TokenType::Arrow:		std::cout << "Arrow"; break;
	//	case TokenType::AngularL:	std::cout << "AngularL"; break;
	//	case TokenType::AngularR:	std::cout << "AngularR"; break;
	//	case TokenType::Underscore:	std::cout << "Underscore"; break;
	//	case TokenType::Newline:	std::cout << "Newline"; break;
	//	case TokenType::Eof:		std::cout << "Eof"; finished = true; break;
	//	}
	//	std::cout << "> \tline = <" << output.line << "> \tcol = <" << output.col << "> \t val = <" << output.lexeme << ">\n";
	//}

	Program p;  // create program
	p.Run();    // run program

	LogService::Flush();

	//std::cout << "Log size: " << LogService::GetCurrentDataUsage() << std::endl;
}