#include "ProgramContextManager.h"

bool ProgramContextManager::LaunchContext(ProgramContext context)
{
	constexpr std::string_view functionName{ "LaunchContext" };

	LogService::Log(LogType::TRACE, className, functionName, "Launching context [" + std::to_string((int)context) + "]");
	switch (context) {
	case ProgramContext::Debug_Error:
	{
		// DEBUG ERROR /////////////////////////////////////////////////////////////////////////////
		LogService::Log(LogType::WIP, className, functionName, "Error context not yet implemented");
	}
	break;
	case ProgramContext::Debug_LogManager:
	{
		// DEBUG LOG MANAGER ////////////////////////////////////////////////////////////////////////////
		LogService::Log(LogType::WIP, className, functionName, "LogManager context not yet implemented");
	}
	break;
	case ProgramContext::Core_ProjectPicker:
	{
		// CORE PROJECT PICKER /////////////////////////////////////////////////////////////////////////////
		LogService::Log(LogType::WIP, className, functionName, "ProjectPicker context not yet implemented");
	}
	break;
	case ProgramContext::Core_ProjectEditor:
	{
		// CORE PROJECT EDITOR /////////////////////////////////////////////////////////////////////////////
		LogService::Log(LogType::WIP, className, functionName, "ProjectEditor context not yet implemented");
	}
	break;
	case ProgramContext::Dev_GuiTestingPane:
	{
		// TEST GUI TESTING PANE ////////////////////////////////////////////////////////////////////////////
		LogService::Log(LogType::WIP, className, functionName, "GuiTestingPane context not yet implemented");
	}
	break;
	default:
		LogService::Log(LogType::ERROR, className, functionName, "Invalid context requested, launch failed");
	}

	return false;
}
