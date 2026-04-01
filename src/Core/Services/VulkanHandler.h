#pragma once

#include "LogService.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//#include <vulkan/vulkan.h>
#include <string>
#include <vector> // needed to build for MacOS

struct QueueFamilyIndices {
	// Struct containing the indexes for each supported queue family.
	// Optional is used to indicate whether each is supported or not.
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool TheyAllExist() {
		return
			graphicsFamily.has_value() &&
			presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanHandler {
private:
	static inline VkInstance vulkanInstance; // initialised via vkCreateInstance in CreateVulkanInstance() function
	static inline VkDevice logicalDevice; // initialised via vkCreateDevice in Initialise() function

	// queues
	static inline VkQueue graphicsQueue;
	static inline VkQueue presentQueue;

	// utility functions
	static bool IsDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR& surface);
	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR& surface);

	static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR& surface);
	static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

public:
	static VkInstance& GetInstance() {
		if (vulkanInstance == VK_NULL_HANDLE) {
			LogService::Log(LogType::CRITICAL, "VulkanHandler.h", "GetInstance", "VkInstance was not initialised (VK_NULL_HANDLE");
		}
		return vulkanInstance;
	}

	static void CreateVulkanInstance(); // creates VulkanInstance
	static void SetupWindowSurface(GLFWwindow* window, const VkAllocationCallbacks*, VkSurfaceKHR& surface);
	// ^^^ calls creates GLFW surface and setups logical devices and queues
	static void GenerateSwapChains(GLFWwindow* window, VkSurfaceKHR& surface, VkPhysicalDevice physicalDevice); // WIP declaration

	static void Initialise();
	static void Cleanup();

	// ClassName
	static constexpr std::string_view className{ "VulkanHandler" };
};

// Vulkan Notes

// --------------------------------------- --------------------------------------- --------------------------------------- ---------------------------------------
//- VkInstance								::Top-level Vulkan object created once
// ::
// --------------------------------------- --------------------------------------- --------------------------------------- ---------------------------------------
//- VkSurfaceKHR(created from GLFW window)	:: Platform-specific handle to window
// ::
// --------------------------------------- --------------------------------------- --------------------------------------- ---------------------------------------
//- VkPhysicalDevice						:: Handle representing a GPU, choose GPU, query GPU capabilities
// ::
// --------------------------------------- --------------------------------------- --------------------------------------- ---------------------------------------
//- VkDevice + queues						:: Interface to GPU, queue for commands
// ::
// --------------------------------------- --------------------------------------- --------------------------------------- ---------------------------------------
//- VkSwapchainKHR							:: Rotating set of images displayed to screen
// ::
// --------------------------------------- --------------------------------------- --------------------------------------- ---------------------------------------
//- VkImageViews							:: "view" into a vulkan image (eg texture, framebuffer)
// ::
// --------------------------------------- --------------------------------------- --------------------------------------- ---------------------------------------
//- VkRenderPass							:: 
// ::
// --------------------------------------- --------------------------------------- --------------------------------------- ---------------------------------------
//- VkGraphicsPipeline						::
// ::
// --------------------------------------- --------------------------------------- --------------------------------------- ---------------------------------------
//- VkFramebuffers							:: Collection of image views used for a render pass
// ::
// --------------------------------------- --------------------------------------- --------------------------------------- ---------------------------------------
//- VkCommandPool + VkCommandBuffers		::
// ::
// --------------------------------------- --------------------------------------- --------------------------------------- ---------------------------------------
//- VkSemaphore / VkFence					::
// ::
// --------------------------------------- --------------------------------------- --------------------------------------- ---------------------------------------