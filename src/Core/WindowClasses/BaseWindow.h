#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <string>

#include "../Rendering/WindowPresenter.h"
#include "../Rendering/LayoutTypes.h"
#include "../Services/VulkanHandler.h"

// Moved SwapchainData to LayoutTypes
//struct SwapchainData {
//	VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
//	std::vector<VkImage> swapchainImages{};
//	VkFormat swapchainImageFormat{ VK_FORMAT_UNDEFINED };
//	VkExtent2D swapchainExtent{ 0,0 };
//	std::vector<VkImageView> swapchainImageViews{};
//};

struct WindowReturnData {
	bool WindowClosed{ false };
	bool FocusChanged{ false };
	bool UserCommandBreak{ false };
	bool WindowInFocus{ false };
};

class BaseWindow {
private:
	// Constants
	static constexpr int MAX_FRAMES_IN_FLIGHT{ 2 };

	// The swapchain allows us to have multiple image targets that are rendered to in sequence
	// The number of available targets is dependent on the GPU hardware and may vary
	// As an example, it may be in the range [0, 1, 2] as 3 frames is most common
	// We index the swapchain images using the variable imageIndex which is aquired during runtime from vulkan

	// However, we don't render to the full list of swapchain targets 1:1
	// We use MAX_FRAMES_IN_FLIGHT to restrict ourself to rendering to a max of 2 images at a time, possibly leaving some idle
	// currentFrame is incremented manually, and in the hard-coded range [0, 1] based on MAX_FRAMES_IN_FLIGHT

	// Swapchain support querying
	struct SwapchainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
	};

	// References
	VulkanHandler& vulkanHandler;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PERSISTENT RESOURCES
	// Created once in Constructor, destroyed once in Destructor
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// OS & Vulkan Window specifics
	size_t windowId{ 0 };
	GLFWwindow* window{ nullptr };
	VkSurfaceKHR surface{ VK_NULL_HANDLE };

	// Synchronisation Objects
	std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
	// ^^^ Sized to MAX_FRAMES_IN_FLIGHT | indexed by currentFrame
	// ^^^ Used to signal whether he have frames on standby
	// ^^^ Persistent as the size is hard-coded
	std::vector<VkSemaphore> renderFinishedSemaphores;
	// ^^^ Sized to swapchain size | indexed by imageIndex
	// ^^^ Used to track progress of rendering stage for each swapchain image
	// ^^^ Transient as the GPU may change number of swapchain images dependent on refresh result
	std::array<VkFence, MAX_FRAMES_IN_FLIGHT> inFlightFences;
	// ^^^ Sized to MAX_FRAMES_IN_FLIGHT | indexed by currentFrame
	// TODO: What is this used for?
	// Persistent
	uint32_t currentFrame{ 0 };

	// Subsystems
	WindowPresenter presenter;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TRANSIENT RESOURCES
	// Recreated on Resize or Format Change
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	SwapchainData swapchainData;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SETUP HELPERS (Persistent)
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Sync_Create();
	void Sync_Cleanup();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// REFRESH HELPERS (Transient)
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Swapchain_Refresh(); // Root of refresh logic
	void Swapchain_Create(VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE); // Used inside Refresh
	void Swapchain_FinalCleanup(); // Not used by refresh, only in destructor

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SWAPCHAIN QUERIES
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Swapchain helper functions
	SwapchainSupportDetails Swapchain_QuerySupport() const;
	VkSurfaceFormatKHR Swapchain_ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
	VkExtent2D Swapchain_ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GLFW CALLBACKS
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// GLFW Callbacks
	//static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

protected:
	// might not be needed
	GLFWwindow* GetGlfwWindow() const { return window; }

public:
	WindowReturnData Update(); // TODO: update to accept an InputEvent& from WindowManager
	size_t GetId() const { return windowId; }

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