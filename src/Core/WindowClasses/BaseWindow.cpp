#include "BaseWindow.h"

#include "../Services/LogService.h"
#include "../ConsoleColours.h"
#include "../Rendering/LayoutTypes.h"

#include <algorithm>
#include <limits>

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// WINDOW LIFECYCLE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

WindowReturnData BaseWindow::Update() {
	constexpr std::string_view functionName{ "Update" };
	WindowReturnData WRD;

	//LogService::Log(LogType::HIGH, className, functionName, "UPDATE");

	// Check for window close requests
	if (glfwWindowShouldClose(window)) {
		WRD.WindowClosed = true;
		return WRD;
	}

	int width{ 0 };
	int height{ 0 };
	glfwGetFramebufferSize(window, &width, &height);
	if (width == 0 || height == 0) {
		return WRD; // Skip rendering if minimised
	}

	// Render loop
	VkDevice logicalDevice{ vulkanHandler.GetLogicalDevice() };

	// Wait for previous frame to finish
	vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	// Reset fence for frame before aquiring the swapchain image
	vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);

	// Aquire an index for a swapchain image that is free to write to
	uint32_t imageIndex{ 0 };
	VkResult result = vkAcquireNextImageKHR(
		logicalDevice,
		swapchainData.swapchain,
		UINT64_MAX,
		imageAvailableSemaphores[currentFrame],
		VK_NULL_HANDLE,
		&imageIndex
	);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		Swapchain_Refresh();
		return WRD;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to acquire swapchain image");
		throw std::runtime_error("Failed to acquire swapchain image!");
	}

	// Update and Present - handles inputs and returns populated command buffer
	InputEvent placeholderInput{};
	VkCommandBuffer commandBuffer = presenter.UpdateAndPresent(placeholderInput, imageIndex);

	// Submit command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	// Wait semaphores
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	// Command buffer
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	// Signal semaphores
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[imageIndex] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(vulkanHandler.GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to submit draw command buffer");
		throw std::runtime_error("Failed to submit draw command buffer!");
	}

	// Present image to window
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	// Swapchains
	VkSwapchainKHR swapchains[] = { swapchainData.swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(vulkanHandler.GetPresentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		// Swapchain became invalid during presentation
		//Swapchain_Refresh(); // <- causes a rendering bug on resize, just wait for next frame
	}
	else if (result != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to present swapchain image");
		throw std::runtime_error("Failed to present swapchain image!");
	}

	// Advance to next frame
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	return WRD;
}

BaseWindow::BaseWindow(size_t id, VulkanHandler& vk, int width, int height, std::string_view title) :
	vulkanHandler(vk),
	presenter(vk),
	windowId(id)
{
	constexpr std::string_view functionName{ "Constructor" };

	LogService::Log(LogType::TRACE, className, functionName,
		"Creating window with id: [" +
		ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT) +
		std::to_string(windowId) +
		ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT) + "]"
	);

	// Create GLFW window
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
	if (!window) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create GLFW window");
		throw std::runtime_error("Failed to create GLFW window!");
	}

	glfwSetWindowUserPointer(window, this);
	LogService::Log(LogType::SUCCESS, className, functionName, "GLFW window created");

	// Create vulkan surface
	if (glfwCreateWindowSurface(vulkanHandler.GetInstance(), window, nullptr, &surface) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create window surface");
		throw std::runtime_error("Failed to create window surface!");
	}

	// Ensure global vulkan devices are initialised
	vulkanHandler.InitialiseDevices(surface);

	// Create persistent synchronisation objects
	Sync_Create();

	// Initialise swapchain for the first time
	Swapchain_Refresh();

	// Initialise window presenter
	presenter.Refresh(swapchainData);

	LogService::Log(LogType::SUCCESS, className, functionName, "Window initialised");
}

BaseWindow::~BaseWindow() {
	constexpr std::string_view functionName{ "Destructor" };

	LogService::Log(LogType::TRACE, className, functionName,
		"Destroying window :: id = [" +
		ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT) +
		std::to_string(windowId) +
		ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT) + "] "
	);

	vkDeviceWaitIdle(vulkanHandler.GetLogicalDevice());

	Sync_Cleanup();
	Swapchain_FinalCleanup();

	if (surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(vulkanHandler.GetInstance(), surface, nullptr);
	}

	glfwDestroyWindow(window);

	LogService::Log(LogType::SUCCESS, className, functionName, "Window destroyed successfully");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// PERSISTENT HELPERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void BaseWindow::Sync_Create() {
	constexpr std::string_view functionName{ "Sync_Create" };

	VkDevice device{ vulkanHandler.GetLogicalDevice() };

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; i++) {
		// Create imageAvailable semaphores
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS) {
			LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create imageAvailable semaphore");
			throw std::runtime_error("Failed to create image available semaphore!");
		}

		// RenderFinished semaphores are transient and dealt with in refresh

		// In-flight fences
		if (vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create fences");
			throw std::runtime_error("Failed to create fences!");
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// TRANSIENT HELPERS (REFRESH)
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void BaseWindow::Swapchain_Refresh() {
	constexpr std::string_view functionName{ "Swapchain_Refresh" };

	VkDevice device{ vulkanHandler.GetLogicalDevice() };
	LogService::Log(LogType::TRACE, className, functionName, "Refreshing swapchain");

	// Wait for GPU to finish using current resources
	vkDeviceWaitIdle(device);

	// Cleanup old imageViews
	for (auto imageView : swapchainData.swapchainImageViews) {
		if (imageView != VK_NULL_HANDLE) { vkDestroyImageView(device, imageView, nullptr); }
	}
	swapchainData.swapchainImageViews.clear();

	// Cleanup old renderFinished semaphores
	for (auto sem : renderFinishedSemaphores) {
		if (sem != VK_NULL_HANDLE) { vkDestroySemaphore(device, sem, nullptr); }
	}
	renderFinishedSemaphores.clear();

	// Create new swapchain from old handle
	VkSwapchainKHR oldSwapchain{ swapchainData.swapchain };
	Swapchain_Create(oldSwapchain);

	// Destroy old swapchain
	if (oldSwapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
	}

	// Create new renderFinished semaphores
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	renderFinishedSemaphores.resize(swapchainData.swapchainImages.size());

	for (size_t i = 0; i < swapchainData.swapchainImages.size(); i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) {
			LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create renderFinished semaphore for swapchain");
			throw std::runtime_error("Failed to create render finished semaphore!");
		}
	}

	// Propogate refresh up to presenter
	presenter.Refresh(swapchainData);
}

void BaseWindow::Swapchain_FinalCleanup() {
	VkDevice device{ vulkanHandler.GetLogicalDevice() };

	// Destroy transient renderFinished semaphores
	for (auto sem : renderFinishedSemaphores) {
		if (sem != VK_NULL_HANDLE) { vkDestroySemaphore(device, sem, nullptr); }
	}
	renderFinishedSemaphores.clear();

	// Destroy imageViews first as they are a dependency of swapchain
	for (auto imageView : swapchainData.swapchainImageViews) {
		if (imageView != VK_NULL_HANDLE) {
			vkDestroyImageView(device, imageView, nullptr);
		}
	}
	swapchainData.swapchainImageViews.clear();

	// Destroy Swapchain
	if (swapchainData.swapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(device, swapchainData.swapchain, nullptr);
		swapchainData.swapchain = VK_NULL_HANDLE;
	}
}

void BaseWindow::Swapchain_Create(VkSwapchainKHR oldSwapchain) {
	constexpr std::string_view functionName{ "Swapchain_Create" };

	// Query capabilites
	SwapchainSupportDetails swapchainSupport = Swapchain_QuerySupport();
	VkSurfaceFormatKHR surfaceFormat = Swapchain_ChooseSurfaceFormat(swapchainSupport.formats);
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // hard-coded support only for FIFO (MAILBOX is ignored)
	VkExtent2D extent = Swapchain_ChooseExtent(swapchainSupport.capabilities);

	uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
	if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
		imageCount = swapchainSupport.capabilities.maxImageCount;
	}

	// Create swapchain
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
	createInfo.oldSwapchain = oldSwapchain;

	if (vkCreateSwapchainKHR(vulkanHandler.GetLogicalDevice(), &createInfo, nullptr, &swapchainData.swapchain) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create swapchain");
		throw std::runtime_error("Failed to create swapchain!");
	}

	// Retrieve images
	vkGetSwapchainImagesKHR(vulkanHandler.GetLogicalDevice(), swapchainData.swapchain, &imageCount, nullptr);
	swapchainData.swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(vulkanHandler.GetLogicalDevice(), swapchainData.swapchain, &imageCount, swapchainData.swapchainImages.data());

	swapchainData.swapchainImageFormat = surfaceFormat.format;
	swapchainData.swapchainExtent = extent;

	// Create image views
	swapchainData.swapchainImageViews.resize(swapchainData.swapchainImages.size());

	for (size_t i{ 0 }; i < swapchainData.swapchainImages.size(); i++) {
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
// SWAPCHAIN QUERIES
//////////////////////////////////////////////////////////////////////////////////////////////////////////

BaseWindow::SwapchainSupportDetails BaseWindow::Swapchain_QuerySupport() const {
	SwapchainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanHandler.GetPhysicalDevice(), surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanHandler.GetPhysicalDevice(), surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanHandler.GetPhysicalDevice(), surface, &formatCount, details.formats.data());
	}

	return details;
}

VkSurfaceFormatKHR BaseWindow::Swapchain_ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkExtent2D BaseWindow::Swapchain_ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent; 	// If not max, the window is not resizable (shouldn't ever happen here)
	}
	else {
		int width{ 0 };
		int height{ 0 };
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLEANUP HELPERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void BaseWindow::Sync_Cleanup() {
	VkDevice device{ vulkanHandler.GetLogicalDevice() };

	for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; i++) {
		// Cleanup imageAvailable semaphore
		if (imageAvailableSemaphores[i] != VK_NULL_HANDLE) {
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		}

		// Cleanup fences
		if (inFlightFences[i] != VK_NULL_HANDLE) {
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}
	}
}