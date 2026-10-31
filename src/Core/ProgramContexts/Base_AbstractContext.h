#pragma once

#include "../Services/WindowManager.h"

class Base_AbstractContext {
private:
	WindowManager& windowManager;

protected:
	Base_AbstractContext(WindowManager& wm) : windowManager(wm) {}
	WindowManager& GetWindowManager() const { return windowManager; }

public:
	virtual ~Base_AbstractContext() = default;

	// Class Name
	static constexpr std::string_view className{ "Base_AbstractContext" };
};