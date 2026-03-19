#include "VulkanHandler.h"

#include "LogService.h"
#include <GLFW/glfw3.h>
#include <map>
#include <set>
#include <vector> // needed to build for MacOS

#include "../ConsoleColours.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

QueueFamilyIndices VulkanHandler::FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR& surface) {
	QueueFamilyIndices indices;

	// QUEUE FAMILIES

	// count number of queues for queried device
	uint32_t queueFamilyCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	// enumerate queue families belonging to queried device
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {

		// graphics
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) { // check if queueFamily is GRAPHICS
			indices.graphicsFamily = i; // store GRAPHICS index to struct
		}

		// present
		VkBool32 presentSupport;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
		if (presentSupport == VK_TRUE) {
			indices.presentFamily = i;
		}

		// family index increment
		i++;
	}

	// Not all indices are guaranteed to be found, rely on IsDeviceSuitable to confirm
	return indices;
}

bool VulkanHandler::IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR& surface) {
	constexpr std::string_view functionName{ "IsDeviceSuitable" };

	// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families

	bool suitable{ true }; // default true unless proven false

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GET DEVICE DATA
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// DEVICE PROPERTIES
	VkPhysicalDeviceProperties deviceProperties; // https://docs.vulkan.org/refpages/latest/refpages/source/VkPhysicalDeviceProperties.html
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	// DEVICE FEATURES
	VkPhysicalDeviceFeatures deviceFeatures; // https://docs.vulkan.org/refpages/latest/refpages/source/VkPhysicalDeviceFeatures.html
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	// DEVICE QUEUES
	QueueFamilyIndices indices = FindQueueFamilies(device, surface);

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

	if (!indices.TheyAllExist()) {
		LogService::Log(LogType::FAIL, className, functionName, "GPU lacks graphics queue family");
		suitable = false;
	}
	else {
		LogService::Log(LogType::SUCCESS, className, functionName, "GPU has graphics queue family");
	}
	LogService::Log(LogType::WIP, className, functionName,
		"Missing checks for present, look into the QueueFamilyIndices struct in tutorial"
	);

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

void VulkanHandler::CreateVulkanInstance()
{
	constexpr std::string_view functionName{ "CreateVulkanInstance" };

	LogService::Log(LogType::TRACE, className, functionName, "0 - Initialising Vulkan");

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

		// Layers <- what do they do?
		instanceCreateInfo.enabledLayerCount = 0; // number of global layers to enable
		LogService::Log(LogType::TRACE, className, functionName, "Vulkan layers: none <- what are these for?");
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
void VulkanHandler::SetupWindowSurface(GLFWwindow* window, const VkAllocationCallbacks* callbacks, VkSurfaceKHR& surface)
{
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

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Choose Physical Device
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	LogService::Log(LogType::TRACE, className, functionName, "Choosing from available physical devices");
	VkPhysicalDevice chosenPhysicalDevice{ VK_NULL_HANDLE }; // handle for a GPU, defaults to null
	{
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
		for (const auto& device : devices) {
			if (IsDeviceSuitable(device, surface)) { // suitability check doesn't do anything yet
				chosenPhysicalDevice = device; // select gpu to use for application
				break;
			}
		}

		// Ensure at least one valid GPU was found, and handle if none
		if (chosenPhysicalDevice == VK_NULL_HANDLE) {
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

	QueueFamilyIndices indices = FindQueueFamilies(chosenPhysicalDevice, surface);

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

	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.enabledLayerCount = 0;
	// validation layers are not setup (see documentation)
		// https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Logical_device_and_queues
		// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers

	// This is where logical device is created
	if (vkCreateDevice(chosenPhysicalDevice, &deviceCreateInfo, nullptr, &logicalDevice) == VK_SUCCESS) {
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

	GenerateSwapChains(window);

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

void VulkanHandler::GenerateSwapChains(GLFWwindow* window) {

	// https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
	//https://vulkan-tutorial.com/Drawing_a_triangle/Swap_chain_recreation
}

void VulkanHandler::Cleanup()
{
	// Window surfaces are destroyed in the BaseWindow destructor
	// VulkanHandler::Cleanup() is only called after all windows are closed
	LogService::Log(LogType::WIP, className, "Cleanup", "Check with WindowManager to enforce all window closure before cleanup");
	LogService::Log(LogType::TRACE, className, "Cleanup", "Cleaning Logical Device");
	vkDestroyDevice(logicalDevice, nullptr);
	LogService::Log(LogType::TRACE, className, "Cleanup", "Cleaning Vulkan Instance");
	vkDestroyInstance(vulkanInstance, nullptr);
}