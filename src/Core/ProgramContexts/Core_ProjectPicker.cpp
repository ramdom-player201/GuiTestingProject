#include "Core_ProjectPicker.h"

Core_ProjectPicker::Core_ProjectPicker(WindowManager& wm) : Base_AbstractContext(wm) {
	LogService::Log(LogType::TRACE, className, "Constructor", "Creating Core_ProjectPicker");
	windowId = GetWindowManager().CreateWindow("Core_ProjectPicker", this);
}