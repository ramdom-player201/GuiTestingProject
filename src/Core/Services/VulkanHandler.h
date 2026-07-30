#pragma once

#include "LogService.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//#include <vulkan/vulkan.h>
#include <string>
#include <vector> // needed to build for MacOS

struct SwapChainData {
	VkSwapchainKHR swapChain{ VK_NULL_HANDLE };
	std::vector<VkImage> swapChainImages{};
	VkFormat swapChainImageFormat{ VK_FORMAT_UNDEFINED };
	VkExtent2D swapChainExtent{ 0,0 };
	std::vector<VkImageView> swapChainImageViews{};
};

class VulkanHandler {
private:

	// structs
	struct QueueFamilyIndices {
		// Struct containing the indexes for each supported queue family.
		// Optional is used to indicate whether each is supported or not.
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool TheyAllExist() const {
			return
				graphicsFamily.has_value() &&
				presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities{}; //https://docs.vulkan.org/refpages/latest/refpages/source/VkSurfaceCapabilitiesKHR.html
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	// member variables
	VkInstance vulkanInstance{ VK_NULL_HANDLE }; // initialised via vkCreateInstance in CreateVulkanInstance() function
	VkDevice logicalDevice{ VK_NULL_HANDLE }; // initialised via vkCreateDevice in Initialise() function
	VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE }; // handle for a GPU, defaults to null
	VkQueue graphicsQueue{ VK_NULL_HANDLE };
	VkQueue presentQueue{ VK_NULL_HANDLE };

	// utility functions
	bool IsDeviceSuitable(VkPhysicalDevice deviceToCheck, const VkSurfaceKHR& surface) const;
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice deviceToCheck, const VkSurfaceKHR& surface) const;

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice, const VkSurfaceKHR& surface) const;
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) const;

public:
	// Safety locks to prevent copy/move/reassignment
	VulkanHandler() = default;
	VulkanHandler(const VulkanHandler&) = delete;
	VulkanHandler& operator=(const VulkanHandler&) = delete;
	VulkanHandler(VulkanHandler&&) = delete;
	VulkanHandler& operator=(VulkanHandler&&) = delete;

	// Getters, since some vulkan stuff is used in BaseWindow and Renderer
	VkInstance GetInstance() const {
		if (vulkanInstance == VK_NULL_HANDLE) {
			LogService::Log(LogType::CRITICAL, className, "GetInstance", "VkInstance was not initialised (VK_NULL_HANDLE)");
		}
		return vulkanInstance;
	}
	 VkDevice GetLogicalDevice() const {
		LogService::Log(LogType::WIP, className, "GetLogicalDevice",
			"If logical device is shared between windows, but not guaranteed, should we require surface to be passed to ensure correct LogicalDevice is always obtained?"
		);
		// logicalDevice is supposed to be shared, and should never be recreated since the requirements are the same for all windows
		if (logicalDevice == VK_NULL_HANDLE) {
			LogService::Log(LogType::CRITICAL, className, "GetLogicalDevice", "No logical device exists (VK_NULL_HANDLE)");
		}
		return logicalDevice;
	}
	const VkPhysicalDevice& GetPhysicalDevice() const {
		if (physicalDevice == VK_NULL_HANDLE) {
			LogService::Log(LogType::CRITICAL, className, "GetPhysicalDevice", "PhysicalDevice not created (VK_NULL_HANDLE)");
		}
		return physicalDevice;
	}
	VkQueue GetGraphicsQueue() const {
		if (graphicsQueue == VK_NULL_HANDLE) {
			LogService::Log(LogType::CRITICAL, className, "GetGraphicsQueue", "Graphics queue not created (VK_NULL_HANDLE)");
		}
		return graphicsQueue;
	}
	VkQueue GetPresentQueue() const {
		if (presentQueue == VK_NULL_HANDLE) {
			LogService::Log(LogType::CRITICAL, className, "GetPresentQueue", "Present queue not created (VK_NULL_HANDLE)");
		}
		return presentQueue;
	}

	void CreateVulkanInstance(); // creates VulkanInstance
	void SetupWindowSurface(GLFWwindow* window, const VkAllocationCallbacks*, VkSurfaceKHR& surface, SwapChainData& swapChainData);
	// ^^^ calls creates GLFW surface and setups logical devices and queues
	void GenerateSwapChains(GLFWwindow* window, VkSurfaceKHR& surface, SwapChainData& swapChainData); // WIP declaration
	void CreateImageViews(SwapChainData& swapChainData);

	//static void Initialise(); // <- not used, since CreateVulkanInstance() is called first
	// consider making initialise call CreateVulkanInstance, purely for keeping things standardised across classes
	//static void Cleanup();
	~VulkanHandler(); // replace cleanup with destructor

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





	// Steps:
	// 1 load vulkan loader and extensions						[/]
	// 2 create VkInstanc if null								[/]
	// 3 create surface from passed window handle and callbacks	[/]
	// 4 choose physical device									[/]
	// 5 query queue family										[/]
	// 6 create logical device									[/]
	// 7 create swapchain										[/]

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