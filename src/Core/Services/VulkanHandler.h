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
	VkSurfaceCapabilitiesKHR capabilities; //https://docs.vulkan.org/refpages/latest/refpages/source/VkSurfaceCapabilitiesKHR.html
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanHandler {
private:
	static inline VkInstance vulkanInstance{ VK_NULL_HANDLE }; // initialised via vkCreateInstance in CreateVulkanInstance() function
	static inline VkDevice logicalDevice{ VK_NULL_HANDLE }; // initialised via vkCreateDevice in Initialise() function
	static inline VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE }; // handle for a GPU, defaults to null

	// queues
	static inline VkQueue graphicsQueue;
	static inline VkQueue presentQueue;

	// utility functions
	static bool IsDeviceSuitable(VkPhysicalDevice deviceToCheck, VkSurfaceKHR& surface);
	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice deviceToCheck, VkSurfaceKHR& surface);

	static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR& surface);
	static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

public:
	static const VkInstance& GetInstance() {
		if (vulkanInstance == VK_NULL_HANDLE) {
			LogService::Log(LogType::CRITICAL, className, "GetInstance", "VkInstance was not initialised (VK_NULL_HANDLE)");
		}
		return vulkanInstance;
	}
	static const VkDevice& GetLogicalDevice() {
		LogService::Log(LogType::WIP, className, "GetLogicalDevice",
			"If logical device is shared between windows, but not guaranteed, should we require surface to be passed to ensure correct LogicalDevice is always obtained?"
		);
		if (logicalDevice == VK_NULL_HANDLE) {
			LogService::Log(LogType::CRITICAL, className, "GetLogicalDevice", "No logical device exists (VK_NULL_HANDLE)");
		}
		return logicalDevice;
	}

	static void CreateVulkanInstance(); // creates VulkanInstance
	static void SetupWindowSurface(GLFWwindow* window, const VkAllocationCallbacks*, VkSurfaceKHR& surface, VkSwapchainKHR& swapChain);
	// ^^^ calls creates GLFW surface and setups logical devices and queues
	static void GenerateSwapChains(GLFWwindow* window, VkSurfaceKHR& surface, VkSwapchainKHR& swapChain); // WIP declaration

	//static void Initialise(); // <- not used, since CreateVulkanInstance() is called first
	// consider making initialise call CreateVulkanInstance, purely for keeping things standardised across classes
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