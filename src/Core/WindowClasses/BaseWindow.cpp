#include "BaseWindow.h"

#include "../Services/LogService.h"
#include "../ConsoleColours.h"

#include <algorithm>
#include <limits>

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// WINDOW LIFECYCLE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

WindowReturnData BaseWindow::Update() {
	constexpr std::string_view functionName{ "Update" };
	WindowReturnData WRD;

	if (glfwWindowShouldClose(window)) {
		WRD.WindowClosed = true;
		return WRD;
	}

	int width{ 0 };
	int height = { 0 };
	glfwGetFramebufferSize(window, &width, &height);
	if (width == 0 || height == 0) {
		return WRD; // Skip rendering if 0 sized or minimised
	}

	// Render loop
	VkDevice logicalDevice = vulkanHandler.GetLogicalDevice();

	// 1 - Wait for previous frame to finish
	vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	// 2 - Reset the fence for this frame before aquiring the swapchain image
	vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);

	// 3 - Acquire the next swapchain image
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(
		logicalDevice,
		swapchainData.swapchain,
		UINT64_MAX,
		imageAvailableSemaphores[currentFrame],
		VK_NULL_HANDLE,
		&imageIndex
	);

	// Handle window resize
	if (result == VK_ERROR_OUT_OF_DATE_KHR || framebufferResized) {
		framebufferResized = false;
		RecreateSwapchain();
		return WRD; // Skip frame to draw on next iteration
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to acquire swap chain image");
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	VkCommandBuffer commandBuffer = compositor.RecordCommands(imageIndex); // review this functon against LayoutCompositor.h

	// 4 - Submit command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	// Note: imageIndex is used because the swapchain holds the semaphore until image is presented
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[imageIndex] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(vulkanHandler.GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to submit draw command buffer");
		throw std::runtime_error("Failed to submit draw command buffer!");
	}

	// 5 - Present image to window
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapchains[] = { swapchainData.swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(vulkanHandler.GetPresentQueue(), &presentInfo);

	// Handle window resize triggered by presentation (such as dragging window edge)
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		RecreateSwapchain();
	}
	else if (result != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to present swap chain image");
		throw std::runtime_error("Failed to present swap chain image!");
	}

	// 6 - Advance the frame counter
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	return WRD;
}

BaseWindow::BaseWindow(size_t id, VulkanHandler& vk, int width, int height, std::string_view title) :
	vulkanHandler(vk),
	compositor(vk),
	windowId(id)
{
	constexpr std::string_view functionName{ "Constructor" };

	LogService::Log(LogType::TRACE, className, functionName,
		"Creating window with id: [" +
		ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT) +
		std::to_string(windowId) +
		ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT) + "]"
	);

	// 1 - Create GLFW Window
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Enable resizing

	window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
	if (!window) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create GLFW window");
		throw std::runtime_error("Failed to create GLFW window!");
	}

	// Set resize callback
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
	LogService::Log(LogType::SUCCESS, className, functionName, "GLFW window created");

	// 2 - Create vulkan surface
	CreateWindowSurface();

	// 3 - Initialise global vulkan devices (has internal checks to prevent reinitialisation; first window to call has priority)
	vulkanHandler.InitialiseDevices(surface);

	// 4 - Create window-specific rendering objects
	CreateSwapchain();
	CreateImageViews();
	compositor.CreateCompositeResources(swapchainData.swapchainImageFormat);
	compositor.CreateFramebuffers(swapchainData.swapchainImageViews, swapchainData.swapchainExtent); // review this functon against LayoutCompositor.h

	// 5 - Create synchronisation objects
	CreateSyncObjects(swapchainData.swapchainImages.size());

	LogService::Log(LogType::SUCCESS, className, functionName, "Window fully initialised");
}

BaseWindow::~BaseWindow() {
	constexpr std::string_view functionName{ "Destructor" };

	LogService::Log(LogType::TRACE, className, functionName,
		"Destroying window :: id = [" +
		ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT) +
		std::to_string(windowId) +
		ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT) + "] "
	);

	// Ensure GPU is idle before destroying
	vkDeviceWaitIdle(vulkanHandler.GetLogicalDevice());

	compositor.CleanupFramebuffers(); // review this functon against LayoutCompositor.h
	compositor.CleanupCompositeResources(); // review this functon against LayoutCompositor.h

	CleanupSyncObjects();
	CleanupSwapchain();

	if (surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(vulkanHandler.GetInstance(), surface, nullptr);
	}

	glfwDestroyWindow(window);

	LogService::Log(LogType::SUCCESS, className, functionName, "Window destroyed successfully");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// VULKAN SURFACE & SWAPCHAIN SETUP
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void BaseWindow::CreateWindowSurface() {
	constexpr std::string_view functionName{ "CreateWindowSurface" };

	if (glfwCreateWindowSurface(vulkanHandler.GetInstance(), window, nullptr, &surface) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create window surface");
		throw std::runtime_error("Failed to create window surface!");
	}
}

void BaseWindow::CreateSwapchain() {
	constexpr std::string_view functionName{ "CreateSwapchain" };

	SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport();

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapchainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapchainSupport.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapchainSupport.capabilities);

	uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
	if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
		imageCount = swapchainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;

	createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(vulkanHandler.GetLogicalDevice(), &createInfo, nullptr, &swapchainData.swapchain) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create swap chain");
		throw std::runtime_error("Failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(vulkanHandler.GetLogicalDevice(), swapchainData.swapchain, &imageCount, nullptr);
	swapchainData.swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(vulkanHandler.GetLogicalDevice(), swapchainData.swapchain, &imageCount, swapchainData.swapchainImages.data());

	swapchainData.swapchainImageFormat = surfaceFormat.format;
	swapchainData.swapchainExtent = extent;
}

void BaseWindow::CreateImageViews() {
	constexpr std::string_view functionName{ "CreateImageViews" };

	swapchainData.swapchainImageViews.resize(swapchainData.swapchainImages.size());

	for (size_t i{ 0 }; i < swapchainData.swapchainImages.size();i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapchainData.swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapchainData.swapchainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(vulkanHandler.GetLogicalDevice(), &createInfo, nullptr, &swapchainData.swapchainImageViews[i]) != VK_SUCCESS) {
			LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create image views");
			throw std::runtime_error("Failed to create image views!");
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYNCHRONISATION & RESIZE HANDLING
//////////////////////////////////////////////////////////////////////////////////////////////////////////


void BaseWindow::CreateSyncObjects(uint32_t imageCount) {
	constexpr std::string_view functionName{ "CreateSyncObjects" };

	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(imageCount);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	// 1 - Create aquire semaphores (sized to MAX_FRAMES_IN_FLIGHT)
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(vulkanHandler.GetLogicalDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS) {
			LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create acquire semaphores");
			throw std::runtime_error("Failed to create acquire semaphores!");
		}
	}

	// 2 - Create render semaphores (sized to imageCount)
	for (size_t i = 0; i < imageCount; i++) {
		if (vkCreateSemaphore(vulkanHandler.GetLogicalDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) {
			LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create render semaphores");
			throw std::runtime_error("Failed to create render semaphores!");
		}
	}

	// 3 - create fences (sized to MAX_FRAMES_IN_FLIGHT)
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateFence(vulkanHandler.GetLogicalDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create fences");
			throw std::runtime_error("Failed to create fences!");
		}
	}
}

void BaseWindow::RecreateSwapchain() {
	constexpr std::string_view functionName{ "RecreateSwapchain" };

	LogService::Log(LogType::TRACE, className, functionName, "Recreating swapchain due to window resize");

	vkDeviceWaitIdle(vulkanHandler.GetLogicalDevice());

	CleanupSwapchain();
	compositor.CleanupFramebuffers(); // review this functon against LayoutCompositor.h
	compositor.CleanupCompositeResources(); // review this functon against LayoutCompositor.h
	CleanupSyncObjects();

	CreateSwapchain();
	CreateImageViews();
	compositor.CreateCompositeResources(swapchainData.swapchainImageFormat);
	compositor.CreateFramebuffers(swapchainData.swapchainImageViews, swapchainData.swapchainExtent); // review this functon against LayoutCompositor.h
	CreateSyncObjects(swapchainData.swapchainImages.size());
}

void BaseWindow::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto baseWindow = reinterpret_cast<BaseWindow*>(glfwGetWindowUserPointer(window));
	baseWindow->framebufferResized = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// SWAPCHAIN HELPERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

BaseWindow::SwapchainSupportDetails BaseWindow::QuerySwapchainSupport() const {
	SwapchainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanHandler.GetPhysicalDevice(), surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanHandler.GetPhysicalDevice(), surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanHandler.GetPhysicalDevice(), surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanHandler.GetPhysicalDevice(), surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanHandler.GetPhysicalDevice(), surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR BaseWindow::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR BaseWindow::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D BaseWindow::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLEANUP HELPERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void BaseWindow::CleanupSwapchain() {
	VkDevice device = vulkanHandler.GetLogicalDevice();

	for (auto imageView : swapchainData.swapchainImageViews) {
		if (imageView != VK_NULL_HANDLE) {
			vkDestroyImageView(device, imageView, nullptr);
		}
	}
	swapchainData.swapchainImageViews.clear();

	if (swapchainData.swapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(device, swapchainData.swapchain, nullptr);
		swapchainData.swapchain = VK_NULL_HANDLE;
	}
}

void BaseWindow::CleanupSyncObjects() {
	VkDevice device = vulkanHandler.GetLogicalDevice();

	for (size_t i{ 0 }; i < imageAvailableSemaphores.size();i++) {
		if (imageAvailableSemaphores[i] != VK_NULL_HANDLE) {
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		}
	}
	imageAvailableSemaphores.clear();
	for (size_t i{ 0 }; i < renderFinishedSemaphores.size();i++) {
		if (renderFinishedSemaphores[i] != VK_NULL_HANDLE) {
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		}
	}
	renderFinishedSemaphores.clear();
	for (size_t i = 0; i < inFlightFences.size(); i++) {
		if (inFlightFences[i] != VK_NULL_HANDLE) {
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}
	}
	inFlightFences.clear();
}