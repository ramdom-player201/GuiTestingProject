#pragma once

#include "LogService.h"

#include <vulkan/vulkan.h>
#include <string>

class VulkanHandler {
private:
	static inline VkInstance vulkanInstance; // initialised via vkCreateInstance in Initialise() function

	static bool IsDeviceSuitable(VkPhysicalDevice device);
public:
	static VkInstance& GetInstance() {
		if (vulkanInstance == VK_NULL_HANDLE) {
			LogService::Log(LogType::CRITICAL, "VulkanHandler.h", "GetInstance", "VkInstance was not initialised (VK_NULL_HANDLE");
		}
		return vulkanInstance;
	}

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