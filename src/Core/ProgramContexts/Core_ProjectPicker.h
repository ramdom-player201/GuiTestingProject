#pragma once

#include "Base_AbstractContext.h"

class Core_ProjectPicker : public Base_AbstractContext {
private:
	uint32_t windowId{ 0 }; // Stored id of created project selection window
protected:
public:
	Core_ProjectPicker(WindowManager& wm);

	// Class Name
	static constexpr std::string_view className{ "Core_ProjectPicker" };
};