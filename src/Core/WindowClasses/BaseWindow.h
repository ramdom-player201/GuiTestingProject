#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <vector>

#include "../Rendering/LayoutCompositor.h"
#include "../Services/VulkanHandler.h"

struct SwapchainData {
	VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
	std::vector<VkImage> swapchainImages{};
	VkFormat swapchainImageFormat{ VK_FORMAT_UNDEFINED };
	VkExtent2D swapchainExtent{ 0,0 };
	std::vector<VkImageView> swapchainImageViews{};
};

struct WindowReturnData {
	bool WindowClosed{ false };
	bool FocusChanged{ false };
	bool UserCommandBreak{ false };
	bool WindowInFocus{ false };
};

class BaseWindow {
private:
    // Constants
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2; // Standard double-buffering for CPU/GPU sync

	// Swapchain support querying
	struct SwapchainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	// References
	VulkanHandler& vulkanHandler;

	// OS & Vulkan Window specifics
	size_t windowId{ 0 };
	GLFWwindow* window{ nullptr };
	VkSurfaceKHR surface{ VK_NULL_HANDLE };
	SwapchainData swapchainData;
	bool framebufferResized{ false };

	// Synchronisation Objects
	std::vector<VkSemaphore> imageAvailableSemaphores;	// Indexed by currentFrame
	std::vector<VkSemaphore> renderFinishedSemaphores;	// Indexed by imageIndex
	std::vector<VkFence> inFlightFences;				// Indexed by currentFrame
	uint32_t currentFrame{ 0 };

	// Subsystems
	LayoutCompositor compositor;

	// Window specific rendering setup
	void CreateWindowSurface();
	void CreateSwapchain();
	void CreateImageViews();
	void CreateSyncObjects(uint32_t imageCount);

	// Swapchain helper functions
	SwapchainSupportDetails QuerySwapchainSupport() const;
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

	// Cleanup helper
	void CleanupSwapchain();
	void CleanupSyncObjects();
	void RecreateSwapchain();

	// GLFW Callbacks
	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

protected:
	// might not be needed
	GLFWwindow* GetGlfwWindow() const { return window; }

public:
	WindowReturnData Update();
	size_t getId() const { return windowId; }

	// Saftey locks
	BaseWindow() = delete;
	BaseWindow(const BaseWindow&) = delete;
	BaseWindow& operator=(const BaseWindow&) = delete;
	BaseWindow(BaseWindow&&) = delete;
	BaseWindow& operator=(BaseWindow&&) = delete;

	// Instantiate a window
	BaseWindow(size_t id, VulkanHandler& vk, int width, int height, std::string_view title);
	~BaseWindow();

	// ClassName
	static constexpr std::string_view className{ "BaseWindow" };
};