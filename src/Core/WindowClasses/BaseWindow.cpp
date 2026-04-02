#include "BaseWindow.h"

#include "../Services/LogService.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include "../Services/VulkanHandler.h"

#include "../ConsoleColours.h"


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

bool printedExtensions{ false };
BaseWindow::BaseWindow(size_t id) { // Constructor
	constexpr std::string_view functionName{ "Constructor" };

	windowId = id;

	// https://vulkan-tutorial.com/Development_environment

	LogService::Log(LogType::TRACE, className, functionName,
		"Creating window with id: [" +
		ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT) +
		std::to_string(windowId) +
		ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT) + "]"
	);


	// CREATE GLFW WINDOW HINT
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

	// CREATE GLFW WINDOW SURFACE

	VulkanHandler::SetupWindowSurface(window, nullptr, surface, swapChain);

	//VkInstance vkInstance = VulkanHandler::GetInstance();
	//if (glfwCreateWindowSurface(vkInstance, window, nullptr, &surface) == VK_SUCCESS) {
	//	LogService::Log(LogType::SUCCESS, className, functionName, "Created glfw-vulkan window surface");
	//}
	//else {
	//	LogService::Log(LogType::ERROR, className, functionName, "Failed to create window surface");
	//}

	// CHECK EXTENSIONS?

	//uint32_t extensionCount = 0;
	//vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	//
	//if (!printedExtensions) {
	//	std::string extensionsList;
	//	//for (uint32_t i = 0; i < extensionCount; i++) {
	//	//	extensionsList += glfwExtensions[i];
	//	//	if (i < extensionCount - 1) {
	//	//		extensionsList += ", ";
	//	//	}
	//	//}
	//	LogService::Log(LogType::TRACE, className, functionName,
	//		"Vulkan supported extensions: [" + std::to_string(extensionCount) + "]"
	//		+ " ::\n extensions: " + extensionsList
	//	);
	//	printedExtensions = true;
	//}

	// SOMETHING MATHS (WIP)

	glm::mat4 matrix;
	glm::vec4 vec;
	auto test = matrix * vec;
}

BaseWindow::~BaseWindow() {
	LogService::Log(LogType::TRACE, className, "Destructor",
		"Destroying window surface :: id = [" +
		ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT) +
		std::to_string(windowId) +
		ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT) + "]"
	);
	vkDestroySwapchainKHR(VulkanHandler::GetLogicalDevice(), swapChain, nullptr);
	vkDestroySurfaceKHR(VulkanHandler::GetInstance(), surface, nullptr);
	LogService::Log(LogType::TRACE, className, "Destructor",
		"Destroying window :: id = [" +
		ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT) +
		std::to_string(windowId) +
		ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT) + "] "
	);
	glfwDestroyWindow(window);
}