#include "VulkanHandler.h"

#include "LogService.h"
#include <GLFW/glfw3.h>

void VulkanHandler::Initialise() {
	constexpr std::string_view functionName{ "Initialise" };

	LogService::Log(
		LogType::TRACE,
		className,
		functionName,
		"Initialising Vulkan"
	);

	// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance

	// Define VKApplicationInfo

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Define VKInstanceCreateInfo

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	createInfo.enabledLayerCount = 0;

	// Get VKResult

	VkResult result = vkCreateInstance(&createInfo, nullptr, &vulkanInstance);

	if (vkCreateInstance(&createInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
		LogService::Log(
			LogType::CRITICAL,
			className,
			functionName,
			"Failed to Initialise Vulkan"
		);
		throw std::runtime_error("failed to create instance!");
	}
	else {
		LogService::Log(
			LogType::SUCCESS,
			className,
			functionName,
			"Vulkan initialised successfully"
		);
	}

}

void VulkanHandler::Cleanup()
{
	LogService::Log(
		LogType::TRACE,
		className,
		"Cleanup",
		"Cleaning Vulkan instance"
	);
	vkDestroyInstance(vulkanInstance, nullptr);
}
