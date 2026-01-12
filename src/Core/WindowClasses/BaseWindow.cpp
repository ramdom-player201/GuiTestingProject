#include "BaseWindow.h"

WindowReturnData BaseWindow::Update()
{
	return WindowReturnData();
}

BaseWindow::BaseWindow(size_t id) {
	windowId = id;
}

BaseWindow::~BaseWindow() {

}