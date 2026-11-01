#pragma once

#include "Base_AbstractContext.h"

class Dev_GuiTestingPane : public Base_AbstractContext {
private:
	uint32_t windowId{ 0 }; // Stored id of created editor window
protected:
public:
	Dev_GuiTestingPane(WindowManager& wm);

	// Class Name
	static constexpr std::string_view className{ "Dev_GuiTestingPane" };
};