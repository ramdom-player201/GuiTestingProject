#include "BaseWindow.h"

#include "../Services/LogService.h"

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

WindowReturnData BaseWindow::Update()
{
	WindowReturnData WRD;
	//while (!glfwWindowShouldClose(window)) {
	//	glfwPollEvents();
	//}

	if (glfwWindowShouldClose(window)) {
		// return close
		WRD.WindowClosed = true;
	}
	else {
		glfwPollEvents();
	}

	return WRD;
}

BaseWindow::BaseWindow(size_t id) {
	windowId = id;

	// https://vulkan-tutorial.com/Development_environment

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	LogService::Log(
		LogType::TRACE,
		"BaseWindow",
		"Constructor",
		"Vulkan supported extensions: [" + std::to_string(extensionCount) + "] "
	);

	glm::mat4 matrix;
	glm::vec4 vec;
	auto test = matrix * vec;
}

BaseWindow::~BaseWindow() {
	LogService::Log(
		LogType::TRACE,
		"BaseWindow",
		"Destructor",
		"Destroying window :: id = [" + std::to_string(windowId) + "] "
	);
	glfwDestroyWindow(window);
}