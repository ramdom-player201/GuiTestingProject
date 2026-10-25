#include "Viewport.h"
#include "../Services/VulkanHandler.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

Viewport::Viewport(VulkanHandler& vk, std::shared_ptr<Scene> scene):
	vulkanHandler(vk),
	targetScene(scene)
{
	// TODO: Initial resource creation
}

Viewport::~Viewport() {
	VkDevice device = vulkanHandler.GetLogicalDevice();

	if (renderTargetView != VK_NULL_HANDLE) {
		vkDestroyImageView(device, renderTargetView, nullptr);
	}
	if (renderTargetMemory != VK_NULL_HANDLE) {
		vkFreeMemory(device, renderTargetMemory, nullptr);
	}
	if (renderTargetImage != VK_NULL_HANDLE) {
		vkDestroyImage(device, renderTargetImage, nullptr);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void Viewport::Render() {
	// TODO: Managed by a job manager, alternate between two unlocked targets
}

VkImageView Viewport::GetLatestTextureView() {
	// TODO: There should be 3 targets, use the latest target that is complete, and lock it
	// When one target is locked, alternate between the other two

	return renderTargetView;
}

void Viewport::ProcessInput(const InputEvent& event, float localMouseX, float localMouseY)
{
}

void Viewport::Resize(uint32_t newWidth, uint32_t newHeight) {
	// TODO: Recreate images, memory, image views to fit new size
	currentWidth = newWidth;
	currentHeight = newHeight;
}