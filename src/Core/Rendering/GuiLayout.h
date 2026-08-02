#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

#include "LayoutTypes.h" // Required for ViewportLayoutRect and PanelType

// Forward declarations
class VulkanHandler;

class GuiLayout {
public:
	GuiLayout(VulkanHandler& vulkanHandler);
	~GuiLayout();

	// Core rendering interface
	void Render(); // Render on the main thread

	// Texture output for compositor
	VkImageView GetBaseTextureView() const;
	VkImageView GetOverlayTextureView() const;

	// Layout data output for compositor
	std::vector<ViewportLayoutRect> GetViewportLayoutRequests() const;

	// Sizing interface, called by LayoutCompositor when window resizes
	void Resize(uint32_t windowWidth, uint32_t windowHeight);

	// Safety locks
	GuiLayout() = delete;
	GuiLayout(const GuiLayout&) = delete;
	GuiLayout& operator=(const GuiLayout&) = delete;
	GuiLayout(GuiLayout&&) = delete;
	GuiLayout& operator=(GuiLayout&&) = delete;
private:
	VulkanHandler& vulkanHandler;

	uint32_t currentWindowWidth{ 0 };
	uint32_t currentWindowHeight{ 0 };

	// Internal render targets for GUI
	VkImage baseTextureImage{ VK_NULL_HANDLE };
	VkDeviceMemory baseTextureMemory{ VK_NULL_HANDLE };
	VkImageView baseTextureView{ VK_NULL_HANDLE };

	VkImage overlayTextureImage{ VK_NULL_HANDLE };
	VkDeviceMemory overlayTextureMemory{ VK_NULL_HANDLE };
	VkImageView overlayTextureView{ VK_NULL_HANDLE };
};