#include "VulkanHandler.h"

#include "LogService.h"
#include <GLFW/glfw3.h>
#include <map>
#include <vector> // needed to build for MacOS

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////


struct QueueFamilyIndices {
	// Struct containing the indexes for each supported queue family.
	// Optional is used to indicate whether each is supported or not.
	std::optional<uint32_t> graphicsFamily;
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	// QUEUE FAMILIES
	uint32_t queueFamilyCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) { // check if queueFamily is GRAPHICS
			indices.graphicsFamily = i; // store GRAPHICS index to struct
		}
		i++;
	}

	return indices;
}

bool VulkanHandler::IsDeviceSuitable(VkPhysicalDevice device) {
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
	QueueFamilyIndices indices = findQueueFamilies(device);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// LOG MESSAGE
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	{
		std::string outputLog = "Checking suitability of [";
		outputLog += deviceProperties.deviceName;
		outputLog += "]";
		LogService::Log(LogType::TRACE, className, functionName, outputLog);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DO CHECKS HERE
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (!indices.graphicsFamily.has_value()) {
		LogService::Log(LogType::FAIL, className, functionName, "GPU lacks graphics queue family");
		suitable = false;
	}
	else {
		LogService::Log(LogType::SUCCESS, className, functionName, "GPU has graphics queue family");
	}

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

void VulkanHandler::Initialise() {
	constexpr std::string_view functionName{ "Initialise" };

	LogService::Log(LogType::TRACE, className, functionName, "Initialising Vulkan");
	LogService::Log(LogType::WIP, className, functionName, "What happens if initialise is called twice?\n Consider adding debounce");

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
	// Define VKInstanceCreateInfo (uses VkApplicationInfo)
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// https://docs.vulkan.org/refpages/latest/refpages/source/VkInstanceCreateInfo.html

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

	{
		LogService::Log(LogType::TRACE, className, functionName, "Creating VkInstance");
		LogService::Log(LogType::WIP, className, functionName, "Do we need second parameter at some point?");

		VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &vulkanInstance);

		// app metadata, allocation callbacks? and the instance object

		if (result != VK_SUCCESS) {
			LogService::Log(LogType::CRITICAL, className, functionName, "Failed to Initialise Vulkan");
			throw std::runtime_error("failed to create instance!");
		}
		else {
			LogService::Log(LogType::SUCCESS, className, functionName, "Vulkan initialised successfully");
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Choose PhysicalDevice
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families

	VkPhysicalDevice chosenGpu = VK_NULL_HANDLE; // handle for a GPU, defaults to null
	{

		uint32_t deviceCount{ 0 };
		vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr); // count vulkan compatible devices
		LogService::Log(LogType::TRACE, className, functionName,
			"Vulkan found [" + std::to_string(deviceCount) + "] compatible GPUs"
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
		for (const auto& device : devices) {
			// https://docs.vulkan.org/refpages/latest/refpages/source/VkPhysicalDeviceProperties.html
			VkPhysicalDeviceProperties gpuProperties;
			vkGetPhysicalDeviceProperties(device, &gpuProperties);
			std::string outputData;
			outputData += "\nGPU Name = [";
			outputData += gpuProperties.deviceName;
			outputData += "]\nVendor Id = [";
			outputData += std::to_string(gpuProperties.vendorID);
			outputData += "]\nDevice Type = [";
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

		// check features of supported GPUs and choose first
		// https://docs.vulkan.org/refpages/latest/refpages/source/vkEnumeratePhysicalDevices.html
		for (const auto& device : devices) {
			if (IsDeviceSuitable(device)) { // suitability check doesn't do anything yet
				chosenGpu = device; // select gpu to use for application
				break;
			}
		}

		if (chosenGpu == VK_NULL_HANDLE) {
			LogService::Log(LogType::CRITICAL, className, functionName, "Failed to select a suitable GPU");
			throw std::runtime_error("Failed to select a suitable GPU");
		}
		else {
			LogService::Log(LogType::SUCCESS, className, functionName, "GPU identified as suitable");
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create Logical Device
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	QueueFamilyIndices indices = findQueueFamilies(chosenGpu);
	{
		LogService::Log(LogType::TRACE, className, functionName, "Creating VkDevice");

		VkDeviceQueueCreateInfo queueCreateInfo{}; // https://docs.vulkan.org/refpages/latest/refpages/source/VkDeviceQueueCreateInfo.html
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		float queuePriority = 1.f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

		deviceCreateInfo.enabledExtensionCount = 0;
		deviceCreateInfo.enabledLayerCount = 0;
		// validation layers are not setup (see documentation)
		// https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Logical_device_and_queues
		// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers

		if (vkCreateDevice(chosenGpu, &deviceCreateInfo, nullptr, &logicalDevice) == VK_SUCCESS) {
			LogService::Log(LogType::SUCCESS, className, functionName, "Vulkan Logical Device Created");
		}
		else {
			LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create Vulkan Logical Device");
			throw std::runtime_error("Failed to create Vulkan Logical Device!");
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SETUP QUEUES
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	{
		vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// END OF INITIALISE()
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanHandler::Cleanup()
{
	LogService::Log(LogType::TRACE, className, "Cleanup", "Cleaning Logical Device");
	vkDestroyDevice(logicalDevice, nullptr);
	LogService::Log(LogType::TRACE, className, "Cleanup", "Cleaning Vulkan Instance");
	vkDestroyInstance(vulkanInstance, nullptr);
}