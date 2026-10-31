#pragma once

#include "Services/VulkanHandler.h"
#include "Services/WindowManager.h"
#include "Services/ProgramContextManager.h"

#include <string>

class Program {
private:
	VulkanHandler vulkanHandler; // must outlive windowManager, create first and destroy last
	WindowManager windowManager; // Depends on VulkanHandler
	ProgramContextManager contextManager; // Depends on WindowManager
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