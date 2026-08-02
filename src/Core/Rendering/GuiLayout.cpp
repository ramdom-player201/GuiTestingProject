#include "GuiLayout.h"

#include "../Services/VulkanHandler.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

GuiLayout::GuiLayout(VulkanHandler& vulkanHandler):vulkanHandler(vulkanHandler){
	// TODO: Create initial base and overlay textures
}

GuiLayout::~GuiLayout() {
	VkDevice device = vulkanHandler.GetLogicalDevice();

	// Cleanup base texture
	if (baseTextureView != VK_NULL_HANDLE) { vkDestroyImageView(device, baseTextureView, nullptr); }
	if (baseTextureMemory != VK_NULL_HANDLE) { vkFreeMemory(device, baseTextureMemory, nullptr); }
	if (baseTextureImage != VK_NULL_HANDLE) { vkDestroyImage(device, baseTextureImage, nullptr); }

	// Cleanup overlay texture
	if (overlayTextureView != VK_NULL_HANDLE) { vkDestroyImageView(device, overlayTextureView, nullptr); }
	if (overlayTextureMemory != VK_NULL_HANDLE) { vkFreeMemory(device, overlayTextureMemory, nullptr); }
	if (overlayTextureImage != VK_NULL_HANDLE) { vkDestroyImage(device, overlayTextureImage, nullptr); }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiLayout::Render()
{
	// TODO: Process UI logic and draw 2D elements to images'
	if (rootWidget) {
		rootWidget->Render();
	}
}

InputEventResult GuiLayout::ProcessInput(const InputEvent& event)
{
	if(rootWidget) {
		DrawRect windowRect{
			0.0f,0.0f,
			static_cast<float>(currentWindowWidth),
			static_cast<float>(currentWindowHeight)
		};
		return rootWidget->ProcessInput(event, windowRect);
	}
	return InputEventResult(); // Return empty if no gui loaded
}

VkImageView GuiLayout::GetBaseTextureView() const
{
	return baseTextureView;
}

VkImageView GuiLayout::GetOverlayTextureView() const
{
	return overlayTextureView;
}

const std::unordered_map<uint32_t,DrawRect>& GuiLayout::GetViewportLayoutRequests() const
{
	// TODO: Return actual layout data based on loaded panels
	return viewportLayoutRequests;
}

void GuiLayout::Resize(uint32_t windowWidth, uint32_t windowHeight)
{
	// TODO: Recreate stuff to fit new size
	currentWindowWidth = windowWidth;
	currentWindowHeight = windowHeight;

	if (rootWidget) {
		SetFlag(rootWidget->flags, UiWidgetFlags::LayoutDirty, true);
	}
}
