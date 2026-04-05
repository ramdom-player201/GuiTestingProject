#include "VulkanHandler.h"

#include "LogService.h"
#include <GLFW/glfw3.h>
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
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

QueueFamilyIndices VulkanHandler::FindQueueFamilies(VkPhysicalDevice deviceToCheck, VkSurfaceKHR& surface) {
	QueueFamilyIndices indices;

	// QUEUE FAMILIES

	// count number of queues for queried device
	uint32_t queueFamilyCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(deviceToCheck, &queueFamilyCount, nullptr);

	// enumerate queue families belonging to queried device
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(deviceToCheck, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {

		// graphics
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) { // check if queueFamily is GRAPHICS
			indices.graphicsFamily = i; // store GRAPHICS index to struct
		}

		// present
		VkBool32 presentSupport;
		vkGetPhysicalDeviceSurfaceSupportKHR(deviceToCheck, i, surface, &presentSupport);
		if (presentSupport == VK_TRUE) {
			indices.presentFamily = i;
		}

		// family index increment
		i++;
	}

	// Not all indices are guaranteed to be found, rely on IsDeviceSuitable to confirm
	return indices;
}

SwapChainSupportDetails VulkanHandler::QuerySwapChainSupport(VkPhysicalDevice deviceToCheck, VkSurfaceKHR& surface) {
	constexpr std::string_view functionName{ "QuerySwapChainSupport" };

	// https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain

	SwapChainSupportDetails details;

	// DEVICE CAPABILITIES
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceToCheck, surface, &details.capabilities);

	// SURFACE FORMATS
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(deviceToCheck, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(deviceToCheck, surface, &formatCount, details.formats.data());
	}

	// PRESENT MODE

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(deviceToCheck, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(deviceToCheck, surface, &presentModeCount,
			details.presentModes.data()
		);
	}

	return details;
}

VkSurfaceFormatKHR VulkanHandler::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	constexpr std::string_view functionName{ "ChooseSwapSurfaceFormat" };

	LogService::Log(LogType::WIP, className, functionName, "Can we print full list?");
	for (const auto& theFormat : availableFormats) {
		if (theFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			theFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			LogService::Log(LogType::SUCCESS, className, functionName, "Chosen BGRA_SRGB");
			return theFormat;
		}
	}
	LogService::Log(LogType::FAIL, className, functionName, "Fallback default");
	LogService::Log(LogType::WIP, className, functionName, "Which was it?");
	return availableFormats[0];
}

VkPresentModeKHR VulkanHandler::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	constexpr std::string_view functionName{ "ChooseSwapPresentMode" };

	LogService::Log(LogType::WIP, className, functionName, "Note that mailbox can stress the GPU if framerate is not limited");
	for (const auto& thePresentMode : availablePresentModes) {
		if (thePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			LogService::Log(LogType::SUCCESS, className, functionName, "Gone with Mailbox mode");
			return thePresentMode;
		}
	}
	LogService::Log(LogType::FAIL, className, functionName, "Defaulted to present FIFO mode");
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanHandler::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
	constexpr std::string_view functionName{ "ChooseSwapExtent" };

	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		LogService::Log(LogType::TRACE, className, functionName, "Within limits, use current");
		return capabilities.currentExtent;
	}
	else {
		LogService::Log(LogType::TRACE, className, functionName, "Screen and DPI may differ, calculating new");

		int width{ 0 };
		int height{ 0 };
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(
			actualExtent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width
		);
		actualExtent.height = std::clamp(
			actualExtent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height
		);

		return actualExtent;
	}
}

//bool skipFirst{ true };	// << temporary debug stuff, only second GPU has mailbox support on primary development device
bool VulkanHandler::IsDeviceSuitable(VkPhysicalDevice deviceToCheck, VkSurfaceKHR& surface) {
	constexpr std::string_view functionName{ "IsDeviceSuitable" };

	// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families

	bool suitable{ true }; // default true unless proven false

	//if (skipFirst) {		// << temporary debug stuff, only second GPU has mailbox support on primary development device
	//	skipFirst = false;	// << temporary debug stuff, only second GPU has mailbox support on primary development device
	//	suitable = false;	// << temporary debug stuff, only second GPU has mailbox support on primary development device
	//}						// << temporary debug stuff, only second GPU has mailbox support on primary development device

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GET DEVICE DATA
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// DEVICE PROPERTIES
	VkPhysicalDeviceProperties deviceProperties; // https://docs.vulkan.org/refpages/latest/refpages/source/VkPhysicalDeviceProperties.html
	vkGetPhysicalDeviceProperties(deviceToCheck, &deviceProperties);
	// ^^^ used for debug print to output device name

	// DEVICE FEATURES
	VkPhysicalDeviceFeatures deviceFeatures; // https://docs.vulkan.org/refpages/latest/refpages/source/VkPhysicalDeviceFeatures.html
	vkGetPhysicalDeviceFeatures(deviceToCheck, &deviceFeatures);
	// ^^^ what is this needed for?

	// DEVICE EXTENSIONS
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	}; // we need to test for swapchain support
	LogService::Log(LogType::WIP, className, functionName,
		"DeviceExtensions variable is created twice in separate functions, shouldn't this be only once?"
	);

	// DEVICE QUEUES
	QueueFamilyIndices indices = FindQueueFamilies(deviceToCheck, surface);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// LOG MESSAGE
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	{
		std::string outputLog = "Checking suitability of [";
		outputLog += ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT);
		outputLog += deviceProperties.deviceName;
		outputLog += ConsoleColours::getColourCode(AnsiColours::DEFAULT);
		outputLog += "]";
		LogService::Log(LogType::TRACE, className, functionName, outputLog);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DO CHECKS HERE
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Check queue families
	LogService::Log(LogType::TRACE, className, functionName, "Checking queue families");
	if (!indices.TheyAllExist()) {
		LogService::Log(LogType::FAIL, className, functionName, "GPU lacks required queue families");
		suitable = false;
		return false;
	}
	else {
		LogService::Log(LogType::SUCCESS, className, functionName, "GPU has required queue families");
	}

	// Check device extensions https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain
	LogService::Log(LogType::TRACE, className, functionName, "Checking device extensions");
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(deviceToCheck, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(deviceToCheck, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName); // remove from list if available
		}

		if (!requiredExtensions.empty()) { // run if at least one required is not available
			LogService::Log(LogType::FAIL, className, functionName, "GPU lacks required extensions");
			suitable = false;
			return false;
		}
		else {
			LogService::Log(LogType::SUCCESS, className, functionName, "GPU has required extensions");
		}
	}

	// Check swap chain sufficiency
	LogService::Log(LogType::TRACE, className, functionName, "Checking swap chain adequacy");
	// extension support is required for swap chains, but missing extensions should return early above
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(deviceToCheck, surface);
	if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) {
		LogService::Log(LogType::FAIL, className, functionName, "Swap chain is not adequate");
		suitable = false;
		return false;
	}
	else {
		LogService::Log(LogType::SUCCESS, className, functionName, "Swap chain is adequate");
	}

	// Swap chain settings


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// END OF CHECKS
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	LogService::Log(LogType::WIP, className, functionName, "GPU required features may need to be updated over time");

	// unknown what features are needed at this stage, so return true for any
	return suitable;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanHandler::CreateVulkanInstance() {
	constexpr std::string_view functionName{ "CreateVulkanInstance" };

	LogService::Log(LogType::TRACE, className, functionName, "0 - Initialising Vulkan");

	if (enableValidationLayers && !CheckValidationLayerSupport()) {
		LogService::Log(LogType::ERROR, className, functionName,
			"Validation layers requested, but not available. Check to see if they are enabled in your Vulkan SDK"
		);
	}
	else {
		LogService::Log(LogType::LOW, className, functionName,
			"Validation layers requested and available"
		);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Ensure this only runs once
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (vulkanInstance != VK_NULL_HANDLE) {
		LogService::Log(LogType::ABNORM, className, functionName, "Vulkan was already initialised, why was this called twice?");
		return;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Define VkApplicationInfo
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	LogService::Log(LogType::TRACE, className, functionName, "1 - Defining VkApplicationInfo");

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
	// Define VKInstanceCreateInfo (uses VkApplicationInfo)
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// https://docs.vulkan.org/refpages/latest/refpages/source/VkInstanceCreateInfo.html

	LogService::Log(LogType::TRACE, className, functionName, "2 - Defining VkInstanceCreateInfo");

	VkInstanceCreateInfo instanceCreateInfo{};
	{
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // reflection for struct type
		instanceCreateInfo.pApplicationInfo = &appInfo; // << application metadata

		// Extensions
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions; // null-terminated UTF-8 string
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;	// number of global extensions to enable
		instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;	// array of strings with the names of each extension to enable

		// Debug log extensions
		std::string extensionsList;
		for (uint32_t i = 0; i < glfwExtensionCount; i++) {
			extensionsList += glfwExtensions[i];
			if (i < glfwExtensionCount - 1) {
				extensionsList += ", ";
			}
		}
		LogService::Log(LogType::TRACE, className, functionName,
			"Vulkan extensions needed [" + std::to_string(glfwExtensionCount) + "]"
			+ " ::\n extensions: " + extensionsList
		);

		// Note, if VK_ERROR_INCOMPATIBLE_DRIVER on MacOS, further modification may be required
		// See end of: https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance

		// Vulkan debugging layers
		if (enableValidationLayers && CheckValidationLayerSupport()) {
			instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
			LogService::Log(LogType::TRACE, className, functionName, "Vulkan layers enabled");
		}
		else {
			instanceCreateInfo.enabledLayerCount = 0;
			LogService::Log(LogType::TRACE, className, functionName, "Vulkan layers disabled");
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create VkInstance (stored as member variable) (uses VKInstanceCreateInfo)
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	LogService::Log(LogType::TRACE, className, functionName, "3 - Creating VkInstance");

	{
		LogService::Log(LogType::WIP, className, functionName, "Do we need VkAllocationCallbacks at some point?");

		VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &vulkanInstance);
		// https://docs.vulkan.org/refpages/latest/refpages/source/vkCreateInstance.html
		// const VkInstanceCreateInfo*
		// const VkAllocationCallbacks* <- what's this?
		// VkInstance*

		// app metadata, allocation callbacks? and the instance object

		if (result != VK_SUCCESS) {
			LogService::Log(LogType::CRITICAL, className, functionName, "Failed to Initialise Vulkan");
			throw std::runtime_error("Failed to create vulkan instance!");
		}
		else {
			LogService::Log(LogType::SUCCESS, className, functionName, "Vulkan initialised successfully");
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

bool hasDebugListedPhysicalDevices{ false };
void VulkanHandler::SetupWindowSurface(GLFWwindow* window, const VkAllocationCallbacks* callbacks, VkSurfaceKHR& surface, VkSwapchainKHR& swapChain) {
	constexpr std::string_view functionName{ "SetupWindowSurface" };

	LogService::Log(LogType::TRACE, className, functionName, "Setting up window surface");

	// Ensure vulkanInstance exists, and intialise if missing
	if (vulkanInstance == VK_NULL_HANDLE) {
		LogService::Log(LogType::ERROR, className, functionName, "Vulkan instance was not initialised, attempt to initialise");
		CreateVulkanInstance();
	}
	else {
		LogService::Log(LogType::TRACE, className, functionName, "Vulkan instance is valid, proceeding");
	}

	// Create window surface
	if (glfwCreateWindowSurface(vulkanInstance, window, callbacks, &surface) == VK_SUCCESS) {
		LogService::Log(LogType::SUCCESS, className, functionName, "Created glfw-vulkan window surface");
	}
	else {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create window surface");
	}
	LogService::Log(LogType::WIP, className, functionName, "In which scenarios might this fail, and how might best to handle it?");
	// multi-window project, ideally we should fail to create the window without crashing everything

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Choose Physical Device
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	LogService::Log(LogType::TRACE, className, functionName, "Choosing from available physical devices");

	if (physicalDevice != VK_NULL_HANDLE) {
		LogService::Log(LogType::LOW, className, functionName, "Physical device already selected");
	}
	else {
		LogService::Log(LogType::LOW, className, functionName, "No physical device selected");

		uint32_t deviceCount{ 0 };
		vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr); // count vulkan compatible devices
		LogService::Log(LogType::TRACE, className, functionName,
			"Vulkan found [" +
			ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT) +
			std::to_string(deviceCount) +
			ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT) +
			"] compatible GPUs"
		);

		if (deviceCount == 0) { // log critical and crash if no supported GPUs
			LogService::Log(LogType::CRITICAL, className, functionName,
				"Failed to find GPU with Vulkan support"
			);
			throw std::runtime_error("Failed to find GPU with Vulkan support");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);  // create vector of VkPhysicalDevice objects size deviceCount
		vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data()); // takes array of VkPhysicalDevices and writes info to each

		// debug print all GPUs
		if (!hasDebugListedPhysicalDevices) {
			for (int i = 0; i < devices.size(); i++) {
				// https://docs.vulkan.org/refpages/latest/refpages/source/VkPhysicalDeviceProperties.html
				VkPhysicalDeviceProperties gpuProperties;
				vkGetPhysicalDeviceProperties(devices[i], &gpuProperties);
				std::string outputData = "Check index: [";
				outputData += std::to_string(i + 1);
				outputData += "]";
				outputData += ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT);
				outputData += "\n-> GPU Name = [";
				outputData += gpuProperties.deviceName;
				outputData += "]\n-> Vendor Id = [";
				outputData += std::to_string(gpuProperties.vendorID);
				outputData += "]\n-> Device Type = [";
				switch (gpuProperties.deviceType) { // https://docs.vulkan.org/refpages/latest/refpages/source/VkPhysicalDeviceType.html
				case 0:
					outputData += "OTHER";
					break;
				case 1:
					outputData += "INTEGRATED_GPU";
					break;
				case 2:
					outputData += "DISCRETE_GPU";
					break;
				case 3:
					outputData += "VIRTUAL_GPU";
					break;
				case 4:
					outputData += "CPU";
					break;
				default:
					outputData += "UNDEFINED";
				}
				LogService::Log(LogType::TRACE, className, functionName, outputData);
			}
			hasDebugListedPhysicalDevices = true;
		}
		else {
			LogService::Log(LogType::TRACE, className, functionName, "Already listed physical devices earlier, no need to repeat");
		}


		// Check features of supported GPUs and choose first
		// https://docs.vulkan.org/refpages/latest/refpages/source/vkEnumeratePhysicalDevices.html
		for (const auto& deviceToCheck : devices) {
			if (IsDeviceSuitable(deviceToCheck, surface)) { // suitability check doesn't do anything yet
				physicalDevice = deviceToCheck; // select gpu to use for application
				break;
			}
		}

		// Ensure at least one valid GPU was found, and handle if none
		if (physicalDevice == VK_NULL_HANDLE) {
			LogService::Log(LogType::CRITICAL, className, functionName, "Failed to select a suitable GPU");
			throw std::runtime_error("Failed to select a suitable GPU");
		}
		else {
			LogService::Log(LogType::SUCCESS, className, functionName, "GPU identified as suitable");
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Query queues
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);

	LogService::Log(LogType::TRACE, className, functionName, "Creating Queues");

	// https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Window_surface

	// https://docs.vulkan.org/refpages/latest/refpages/source/VkDeviceQueueCreateInfo.html
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfoVector;
	// it's possible that graphicsFamily and presentFamily have the same value
	std::set<uint32_t> uniqueQueueFamilies =
	{
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

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create Logical Device
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	LogService::Log(LogType::TRACE, className, functionName, "Creating VkDevice (Logical Device)");
	LogService::Log(LogType::WIP, className, functionName,
		"How might we reuse existing logical devices instead of creating new ones for each new window?"
	);
	LogService::Log(LogType::WIP, className, functionName,
		"Queues are owned by Logical Device, logical device can be shared"
	);

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = (uint32_t)queueCreateInfoVector.size();
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfoVector.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	LogService::Log(LogType::WIP, className, functionName,
		"DeviceExtensions variable is created twice in separate functions, shouldn't this be only once?"
	);

	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	deviceCreateInfo.enabledLayerCount = 0;
	// validation layers are not setup (see documentation)
		// https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Logical_device_and_queues
		// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers

	if (logicalDevice != VK_NULL_HANDLE) {
		LogService::Log(LogType::ERROR, className, functionName, "Logical device already exists");
		LogService::Log(LogType::WIP, className, functionName, "Need to reuse Logical device if requested features match");
	}

	// This is where logical device is created
	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice) == VK_SUCCESS) {
		LogService::Log(LogType::SUCCESS, className, functionName, "Vulkan Logical Device Created");
	}
	else {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create Vulkan Logical Device");
		throw std::runtime_error("Failed to create Vulkan Logical Device!");
	}

	// Add queues to created device
	LogService::Log(LogType::TRACE, className, functionName, "Linking queues to VkDevice");
	//https://docs.vulkan.org/refpages/latest/refpages/source/vkGetDeviceQueue.html
	vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WIP
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	LogService::Log(LogType::WIP, className, functionName, "Next step is SwapChains");

	GenerateSwapChains(window, surface, swapChain);

	// called from BaseWindow

	// Steps:
	// 1 load vulkan loader and extensions						[/]
	// 2 create VkInstanc if null								[/]
	// 3 create surface from passed window handle and callbacks	[/]
	// 4 choose physical device									[/]
	// 5 query queue family										[/]
	// 6 create logical device									[/]
	// 7 create swapchain										[

	// Global (Once in handler)
	// VkInstance
	// VkPhysicalDevice (not created)
	// VkDevice (logical device, created after window, but windows still share)
	// VkQueue (owned by device)
	// VkDescriptorSetLayout
	// VkPipelineLayout
	// VkPipeine

	// Per-Window
	// VkSurfaceKHR
	// VkSwapchainKHR
	// VkImage
	// VkImageView
	// VkFramebuffer
	// VkRenderPass
	// VkSemaphore, VkFence

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanHandler::GenerateSwapChains(GLFWwindow* window, VkSurfaceKHR& surface, VkSwapchainKHR& swapChain) {
	constexpr std::string_view functionName{ "GenerateSwapChains" };

	// https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
	//https://vulkan-tutorial.com/Drawing_a_triangle/Swap_chain_recreation

	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice, surface);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, window);

	uint32_t imageCount{ swapChainSupport.capabilities.minImageCount + 1 };
	// clamp image count to maximum if needed
	if (swapChainSupport.capabilities.maxImageCount > 0 &&
		imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	LogService::Log(LogType::TRACE, className, functionName, "Creating SwapchainCreateInfo");

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // << may need changing later

	LogService::Log(LogType::WIP, className, functionName,
		"May need to change ImageUsage to allow for layered/asynchronous rendering in the future"
	);

	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
	uint32_t queueFamilyIndices[]{
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	LogService::Log(LogType::WIP, className, functionName,
		"Evaluate concurrent vs exclusive, what is the difference and which is best?"
	);
	if (indices.graphicsFamily != indices.presentFamily) {
		LogService::Log(LogType::TRACE, className, functionName, "Concurrent");
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		LogService::Log(LogType::TRACE, className, functionName, "Exclusive");
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	// Concurrent is simplest; tutorial suggests using it when available for now
	// Exclusive is for better performance

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	// https://docs.vulkan.org/refpages/latest/refpages/source/VkSurfaceTransformFlagBitsKHR.html

	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	// https://docs.vulkan.org/refpages/latest/refpages/source/VkCompositeAlphaFlagBitsKHR.html

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	if (swapChain == VK_NULL_HANDLE) {
		LogService::Log(LogType::TRACE, className, functionName, "No existing swap chain");
	}
	else {
		LogService::Log(LogType::TRACE, className, functionName, "Swap chain exists");
	}

	createInfo.oldSwapchain = VK_NULL_HANDLE;
	LogService::Log(LogType::WIP, className, functionName, "Swap chains are not currently recreated");

	LogService::Log(LogType::WIP, className, functionName,
		"When might creating a swapchain fail and how could it be handled better if other windows exist?"
	);
	if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create swap chain");
		throw std::runtime_error("Failed to create swap chain");
	}
	else {
		LogService::Log(LogType::SUCCESS, className, functionName, "Successfully created swap chain");
	}

}

void VulkanHandler::Cleanup() {
	// Window surfaces are destroyed in the BaseWindow destructor
	// VulkanHandler::Cleanup() is only called after all windows are closed
	LogService::Log(LogType::WIP, className, "Cleanup", "Check with WindowManager to enforce all window closure before cleanup");
	LogService::Log(LogType::TRACE, className, "Cleanup", "Cleaning Logical Device");
	vkDestroyDevice(logicalDevice, nullptr);
	LogService::Log(LogType::TRACE, className, "Cleanup", "Cleaning Vulkan Instance");
	vkDestroyInstance(vulkanInstance, nullptr);
}