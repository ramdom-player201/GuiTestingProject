#include "Dev_GuiTestingPane.h"

Dev_GuiTestingPane::Dev_GuiTestingPane(WindowManager& wm) : Base_AbstractContext(wm) {
	GetWindowManager().CreateWindow("Dev_GuiTestingPane");
}