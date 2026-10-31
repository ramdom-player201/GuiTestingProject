#pragma once

#include "LogService.h"
#include "WindowManager.h"

#include "../ProgramContexts/Base_AbstractContext.h"

enum class ProgramContext : uint8_t {
	// Debug
	Debug_Error = 0,
	Debug_LogManager = 1,

	// Core system
	Core_ProjectPicker = 10,
	Core_ProjectEditor = 11,

	// Development
	Dev_GuiTestingPane = 100,
};

class ProgramContextManager {
private:
	WindowManager& windowManager;
	std::vector<std::unique_ptr<Base_AbstractContext>> contexts;

public:

	ProgramContextManager(WindowManager& wm) : windowManager(wm) { LogService::Log(LogType::TRACE, className, "Constructor", "Created ProgramContextManager"); }
	~ProgramContextManager() {};

	// Safety locks to prevent copy/move/reassignment
	ProgramContextManager() = delete;
	ProgramContextManager(const ProgramContextManager&) = delete;
	ProgramContextManager& operator=(const ProgramContextManager&) = delete;
	ProgramContextManager(ProgramContextManager&&) = delete;
	ProgramContextManager& operator=(ProgramContextManager&&) = delete;

	bool LaunchContext(ProgramContext context);

	// ClassName
	static constexpr std::string_view className{ "ProgramContextManager" };
};