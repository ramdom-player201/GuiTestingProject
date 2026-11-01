#pragma once

#include "../Services/WindowManager.h"

struct ContextUpdateResult {
	bool contextShouldClose{ false };
	// Room to add additional fields as needed
};

class Base_AbstractContext {
private:
	WindowManager& windowManager;

protected:
	bool contextShouldClose{ false }; // Can be set via callback, or internal close action

	Base_AbstractContext(WindowManager& wm) : windowManager(wm) {}
	WindowManager& GetWindowManager() const { return windowManager; }

public:
	virtual ~Base_AbstractContext() = default;

	virtual ContextUpdateResult Update() { return { contextShouldClose }; }

	virtual bool HandleClosureRequest(uint32_t windowId) {
		contextShouldClose = true;
		return true;
	}

	// Class Name
	static constexpr std::string_view className{ "Base_AbstractContext" };
};

/*
Additional documentation on OnCloseRequested callback

Called by WindowManager when a user requests to close the window used by the Context
True = accept closure, False = veto closure (eg if work is unsaved, veto and present dialog options)

Callback is fired during WindowManager::UpdateWindows()
That function uses a deferred-closure loop, so it is safe to call Create/Close functions from within the callback

By default, Contexts allow BaseWindow closure; this can be changed by overriding the function
It is important that contextShouldClose is correctly set when a closure occurs
*/