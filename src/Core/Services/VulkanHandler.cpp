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

	LogService::Log(LogType::TRACE, className, functionName, "Initialising Vulkan");

	// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Define VkApplicationInfo
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	VkApplicationInfo appInfo{};
	{
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		/*
		VkApplicationInfo Description				// https://docs.vulkan.org/refpages/latest/refpages/source/VkApplicationInfo.html
			VkStructureType    sType;				// reflection to identify the struct
			const void*        pNext;				// null or pointer to an extension struct
			const char*        pApplicationName;	// null or string for name of application
			uint32_t           applicationVersion;	// deverloper supplied version number of application
			const char*        pEngineName;			// null or string containing name of engine
			uint32_t           engineVersion;		// developer supplied version number of engine
			uint32_t           apiVersion;			// highest version of Vulkan the application is designed to use

		Not super important, and can even be passed as null, but may be useful if hardware vendors want to design hardware
		optimisations for certain popular programs. This provides an easy way to identify the application for those
		optimisations.
		https://stackoverflow.com/questions/38256670/what-is-the-point-of-vkapplicationinfo
		*/
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Define VKInstanceCreateInfo
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// https://docs.vulkan.org/refpages/latest/refpages/source/VkInstanceCreateInfo.html

	VkInstanceCreateInfo createInfo{};
	{
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // reflection for struct type
		createInfo.pApplicationInfo = &appInfo; // << application metadata

		// Extensions
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions; // null-terminated UTF-8 string
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		createInfo.enabledExtensionCount = glfwExtensionCount;	// number of global extensions to enable
		createInfo.ppEnabledExtensionNames = glfwExtensions;	// array of strings with the names of each extension to enable

		// Layers <- what do they do?
		createInfo.enabledLayerCount = 0; // number of global layers to enable
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create VkInstance (stored as member variable)
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	{
		LogService::Log(LogType::TRACE, className, functionName, "Creating vkInstance");
		LogService::Log(LogType::WIP, className, functionName,
			"Do we need second parameter at some point?");

		VkResult result = vkCreateInstance(&createInfo, nullptr, &vulkanInstance);

		// app metadata, allocation callbacks? and the instance object

		if (result != VK_SUCCESS) {
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

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Choose PhysicalDevice
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	{
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
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			LogService::Log(LogType::CRITICAL, className, functionName,
				"Failed to select a suitable GPU"
			);
			throw std::runtime_error("Failed to select a suitable GPU");
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// END OF INITIALISE()
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
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
