#include "Dev_GuiTestingPane.h"

Dev_GuiTestingPane::Dev_GuiTestingPane(WindowManager& wm) : Base_AbstractContext(wm) {
	LogService::Log(LogType::TRACE, className, "Constructor", "Creating Dev_GuiTestingPane");
	windowId = GetWindowManager().CreateWindow("Dev_GuiTestingPane", this);
}