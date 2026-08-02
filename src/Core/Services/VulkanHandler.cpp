#include "VulkanHandler.h"

#include "LogService.h"
#include <map>
#include <set>
#include <algorithm>
#include <cstring> // Required for Linux to compile

#include "../ConsoleColours.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Validation layers
const std::vector<const char*> validationLayers{
	"VK_LAYER_KHRONOS_validation"
};
#ifdef NDEBUG
const bool enableValidationLayers{ false };
#else
const bool enableValidationLayers{ true };
#endif

// Device extensions
const std::vector<const char*> deviceExtensions{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

bool CheckValidationLayerSupport() {
	uint32_t layerCount{ 0 };
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound{ false };

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// LOCAL HELPERS FOR DEVICE SELECTION
//////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {
	struct SwapchainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
		SwapchainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

bool VulkanHandler::IsDeviceSuitable(VkPhysicalDevice deviceToCheck, const VkSurfaceKHR& surface) const {
	constexpr std::string_view functionName("IsDeviceSuitable");

	//bool suitable{ true }; <- all fails return false anyway

	// Get device data
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(deviceToCheck, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(deviceToCheck, &deviceFeatures);
	// ^^^ What is this needed for?

	QueueFamilyIndices indices = FindQueueFamilies(deviceToCheck, surface);

	// Log message
	{
		std::string outputLog = "Checking suitability of [";
		outputLog += ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT);
		outputLog += deviceProperties.deviceName;
		outputLog += ConsoleColours::getColourCode(AnsiColours::DEFAULT);
		outputLog += "]";
		LogService::Log(LogType::TRACE, className, functionName, outputLog);
	}

	// CHECKS

	// Check queue families
	LogService::Log(LogType::TRACE, className, functionName, "Checking queue families");
	if (!indices.TheyAllExist()) {
		LogService::Log(LogType::FAIL, className, functionName, "GPU lacks required queue families");
		return false;
	}
	else {
		LogService::Log(LogType::SUCCESS, className, functionName, "GPU has required queue families");
	}

	// Check device extensions
	LogService::Log(LogType::TRACE, className, functionName, "Checking device extensions");
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(deviceToCheck, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(deviceToCheck, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		if (!requiredExtensions.empty()) {
			LogService::Log(LogType::FAIL, className, functionName, "GPU lacks required extensions");
			return false;
		}
		else {
			LogService::Log(LogType::SUCCESS, className, functionName, "GPU has required extensions");
		}
	}

	// Check swapchain sufficiency
	LogService::Log(LogType::TRACE, className, functionName, "Checking swapchain adequacy");
	SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(deviceToCheck, surface);
	if (swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty()) {
		LogService::Log(LogType::FAIL, className, functionName, "Swapchain is not adequate");
		return false;
	}
	else {
		LogService::Log(LogType::SUCCESS, className, functionName, "Swapchain is adequate");
	}

	LogService::Log(LogType::WIP, className, functionName, "GPU required features may need to be updated over time");
	return true;
}

VulkanHandler::QueueFamilyIndices VulkanHandler::FindQueueFamilies(VkPhysicalDevice deviceToCheck, const VkSurfaceKHR& surface) const {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(deviceToCheck, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(deviceToCheck, &queueFamilyCount, queueFamilies.data());

	int i{ 0 };
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport;
		vkGetPhysicalDeviceSurfaceSupportKHR(deviceToCheck, i, surface, &presentSupport);
		if (presentSupport == VK_TRUE) {
			indices.presentFamily = i;
		}

		i++;
	}
	return indices;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanHandler::CreateVulkanInstance(const std::vector<const char*>& requiredExtensions) {
	constexpr std::string_view functionName{ "CreateVulkanInstance" };

	LogService::Log(LogType::TRACE, className, functionName, "0 - Initialising Vulkan");

	if (enableValidationLayers && !CheckValidationLayerSupport()) {
		LogService::Log(LogType::ERROR, className, functionName,
			"Validation layers requested, but not available. Check to see if they are enabled in your Vulkan SDK"
		);
	}
	else {
		LogService::Log(LogType::LOW, className, functionName, "Validation layers requested and available");
	}

	if (vulkanInstance != VK_NULL_HANDLE) {
		LogService::Log(LogType::ABNORM, className, functionName, "Vulkan was already initialised, why was this called twice?");
		return;
	}

	// Define VkApplicationInfo
	LogService::Log(LogType::TRACE, className, functionName, "1 - Defining VkApplicationInfo");
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Define VkInstanceCreateInfo
	LogService::Log(LogType::TRACE, className, functionName, "2 - Defining VkInstanceCreateInfo");
	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	// Extensions
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();

	std::string extensionsList;
	for (size_t i{ 0 }; i < requiredExtensions.size();i++) {
		extensionsList += requiredExtensions[i];
		if (i < requiredExtensions.size() - 1) {
			extensionsList += ", ";
		}
	}
	LogService::Log(LogType::TRACE, className, functionName,
		"Vulkan extensions needed [" + std::to_string(requiredExtensions.size()) + "] ::\n extensions: " + extensionsList
	);

	//Vulkan debugging layers
	if (enableValidationLayers && CheckValidationLayerSupport()) {
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		LogService::Log(LogType::TRACE, className, functionName, "Vulkan layers enabled");
	}
	else {
		instanceCreateInfo.enabledLayerCount = 0;
		LogService::Log(LogType::TRACE, className, functionName, "Vulkan layers disabled");
	}

	// Create VkInstance
	LogService::Log(LogType::TRACE, className, functionName, "3 - Creating VkInstance");
	LogService::Log(LogType::WIP, className, functionName, "Do we need VkAllocationCallbacks at some point?");

	VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &vulkanInstance);
	if (result != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to initialise Vulkan");
		throw std::runtime_error("Failed to create vulkan instance!");
	}
	else {
		LogService::Log(LogType::SUCCESS, className, functionName, "Vulkan initialised successfully");
	}
}

void VulkanHandler::InitialiseDevices(const VkSurfaceKHR& surface)
{
	PickPhysicalDevice(surface);
	CreateLogicalDevice(surface);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanHandler::PickPhysicalDevice(const VkSurfaceKHR& surface) {
	constexpr std::string_view functionName{ "PickPhysicalDevice" };

	LogService::Log(LogType::TRACE, className, functionName, "Choosing from available physical devices");

	if (physicalDevice != VK_NULL_HANDLE) {
		LogService::Log(LogType::LOW, className, functionName, "Physical device already selected");
		return;
	}

	uint32_t deviceCount{ 0 };
	vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);

	LogService::Log(LogType::TRACE, className, functionName,
		"Vulkan found [" +
		ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT) +
		std::to_string(deviceCount) +
		ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT) +
		"] compatible GPUs"
	);

	if (deviceCount == 0) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to find GPU with Vulkan support");
		throw std::runtime_error("Failed to find GPU with Vulkan support");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());

	// Debug print all GPUs
	for (int i{ 0 }; i < devices.size(); i++) {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
		std::string outputData = "Check index: [" + std::to_string(i + 1) + "]";
		outputData += ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT);
		outputData += "\n-> GPU Name = [" + std::string(deviceProperties.deviceName) + "]";
		outputData += "\n-> Vendor Id = [" + std::to_string(deviceProperties.vendorID) + "]";
		outputData += "\n-> Device Type = [";

		switch (deviceProperties.deviceType) {
		case 0: outputData += "OTHER"; break;
		case 1: outputData += "INTEGRATED_GPU"; break;
		case 2: outputData += "DISCRETE_GPU"; break;
		case 3: outputData += "VIRTUAL_GPU"; break;
		case 4: outputData += "CPU"; break;
		default: outputData += "UNDEFINED"; break;
		}
		LogService::Log(LogType::TRACE, className, functionName, outputData);
	}

	// Check features and choose first suitable
	for (const auto& deviceToCheck : devices) {
		if (IsDeviceSuitable(deviceToCheck, surface)) {
			physicalDevice = deviceToCheck;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to select a suitable GPU");
		throw std::runtime_error("Failed to select a suitable GPU");
	}
	else {
		LogService::Log(LogType::SUCCESS, className, functionName, "GPU identified as suitable");
	}
}

void VulkanHandler::CreateLogicalDevice(const VkSurfaceKHR& surface) {
	constexpr std::string_view functionName{ "CreateLogicalDevice" };

	LogService::Log(LogType::TRACE, className, functionName, "Creating VkDevice (LogicalDevice)");

	if (logicalDevice != VK_NULL_HANDLE) {
		LogService::Log(LogType::LOW, className, functionName, "Logical device already exists. Reusing for new window surface.");
		return;
	}

	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
	LogService::Log(LogType::TRACE, className, functionName, "Creating Queues");

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfoVector;
	std::set<uint32_t> uniqueQueueFamilies = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfoVector.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfoVector.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfoVector.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// Use the file-scoped deviceExtensions constant
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	deviceCreateInfo.enabledLayerCount = 0;

	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice) == VK_SUCCESS) {
		LogService::Log(LogType::SUCCESS, className, functionName, "Vulkan Logical Device created");
	}
	else {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create Vulkan Logical Device");
		throw std::runtime_error("Failed to create Vulkan Logical Device!");
	}

	LogService::Log(LogType::TRACE, className, functionName, "Linking queues to VkDevice");

	graphicsQueueFamilyIndex = indices.graphicsFamily.value();

	vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// HARDWARE QUERIES
//////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t VulkanHandler::GetGraphicsQueueFamilyIndex() const {
	constexpr std::string_view functionName{ "GetGraphicsQueueFamilyIndex" };

	if (logicalDevice == VK_NULL_HANDLE) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Logical device not initialised, cannot query queue family index");
	}

	return graphicsQueueFamilyIndex;
}

uint32_t VulkanHandler::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
	constexpr std::string_view functionName{ "FindMemoryType" };

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		// Check if the memory type is compatible with the buffer's requirements (typeFilter)
		// AND if it has the features we want (e.g., CPU visible)
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	LogService::Log(LogType::CRITICAL, className, functionName, "Failed to find suitable memory type");
	throw std::runtime_error("Failed to find suitable memory type!");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanHandler::~VulkanHandler() {
	constexpr std::string_view functionName{ "Destructor" };

	if (logicalDevice != VK_NULL_HANDLE) {
		vkDestroyDevice(logicalDevice, nullptr);
		LogService::Log(LogType::TRACE, className, functionName, "Destroyed Logical Device");
	}

	if (vulkanInstance != VK_NULL_HANDLE) {
		vkDestroyInstance(vulkanInstance, nullptr);
		LogService::Log(LogType::TRACE, className, functionName, "Destroyed Vulkan Instance");
	}
}