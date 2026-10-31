#include "Core_ProjectPicker.h"

Core_ProjectPicker::Core_ProjectPicker(WindowManager& wm) : Base_AbstractContext(wm) {
	GetWindowManager().CreateWindow("Core_ProjectPicker");
}