#include "VulkanHandler.h"

#include "LogService.h"
#include <GLFW/glfw3.h>
#include <map>
#include <vector> // needed to build for MacOS

bool isDeviceSuitable(VkPhysicalDevice device) {

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	// unknown what features are needed at this stage, so return true for any
	return true;
}

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

	// Choose PhysicalDevice

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount{ 0 };
	vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr); // count vulkan compatible devices
	LogService::Log(LogType::TRACE, className, functionName,
		"Vulkan found [" + std::to_string(deviceCount) + "] compatible devices"
	);

	if (deviceCount == 0) { // log critical and crash if no supported GPUs
		LogService::Log(LogType::CRITICAL, className, functionName,
			"Failed to find GPU with Vulkan support"
		);
		throw std::runtime_error("Failed to find GPU with Vulkan support");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount); // check features of supported devices and choose first
	vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());
	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		LogService::Log(LogType::CRITICAL, className, functionName,
			"Failed to select a suitable GPU"
		);
		throw std::runtime_error("Failed to select a suitable GPU");
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
