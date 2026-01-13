#pragma once

#include <string>

class Program {
private:
	
public:
	void Run();

	Program();
	~Program();

	// ClassName
	static constexpr std::string_view className{ "Program" };
};