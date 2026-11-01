#include "ProgramContextManager.h"

#include "../ProgramContexts/Core_ProjectPicker.h"
#include "../ProgramContexts/Dev_GuiTestingPane.h"

bool ProgramContextManager::LaunchContext(ProgramContext contextType)
{
	constexpr std::string_view functionName{ "LaunchContext" };

	LogService::Log(LogType::TRACE, className, functionName, "Launching context [" + std::to_string((int)contextType) + "]");

	std::unique_ptr<Base_AbstractContext> context;

	switch (contextType) {
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
		// CORE PROJECT PICKER //////////////////////////////////////////////////////////////////
		LogService::Log(LogType::TRACE, className, functionName, "Launching Core_ProjectPicker");
		context = std::make_unique<Core_ProjectPicker>(windowManager);
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
		// TEST GUI TESTING PANE //////////////////////////////////////////////////////////////
		LogService::Log(LogType::WIP, className, functionName, "Launching Dev_GuiTestingPane");
		context = std::make_unique<Dev_GuiTestingPane>(windowManager);
	}
	break;
	}

	if (!context) {
		LogService::Log(LogType::ERROR, className, "LaunchContext", "Failed to create a new context");
		return false;
	}

	contexts.push_back(std::move(context));
	return true;
}

void ProgramContextManager::UpdateContexts()
{
	// Loop through all spawned contexts and update them
	for (auto it = contexts.begin(); it != contexts.end();) {
		ContextUpdateResult result{ (*it)->Update() }; // Update context and get result
		if (result.contextShouldClose) {
			it = contexts.erase(it); // If context requests close, erase from vector without incrementing iterator
		}
		else {
			++it; // Iterate to next context
		}
	}
}