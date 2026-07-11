#pragma once

#include <string>
#include "Services/VulkanHandler.h"
#include "Services/WindowManager.h"

class Program {
private:
	VulkanHandler vulkanHandler;
	WindowManager windowManager;
public:
	void Run();

	Program();
	~Program();

	// Safety locks to prevent copy/move/reassignment
	Program(const Program&) = delete;
	Program& operator=(const Program&) = delete;
	Program(Program&&) = delete;
	Program& operator=(Program&&) = delete;

	// ClassName
	static constexpr std::string_view className{ "Program" };
};