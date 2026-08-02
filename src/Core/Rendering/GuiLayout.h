#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <memory>

#include "LayoutTypes.h" // Required for DrawRect (possibly PanelType, but that is stub right now)
#include "UiWidget.h"

// Forward declarations
class VulkanHandler;

enum class PageMode :uint8_t {
	DockableEditor,
	DedicatedViewer,
	SplashScreen
};

class GuiLayout {
public:
	GuiLayout(VulkanHandler& vulkanHandler);
	~GuiLayout();

	// Core rendering interface
	void Render(); // Render on the main thread

	// Handle inputs
	InputEventResult ProcessInput(const InputEvent& event);

	// Texture output for compositor
	VkImageView GetBaseTextureView() const;
	VkImageView GetOverlayTextureView() const;

	// Layout data output for compositor
	const std::unordered_map<uint32_t, DrawRect>& GetViewportLayoutRequests() const;

	// Sizing interface, called by LayoutCompositor when window resizes
	void Resize(uint32_t windowWidth, uint32_t windowHeight);

	// Load gui tree based on app state
	void LoadPage(PageMode mode, const std::string& layoutFilePath);

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

	std::unique_ptr<UiWidget> rootWidget;

	// Cached map for viewport layout data (populated during CalculateLayout by finding ViewportWidgets)
	std::unordered_map<uint32_t, DrawRect> viewportLayoutRequests;

	// Internal render targets for GUI
	VkImage baseTextureImage{ VK_NULL_HANDLE };
	VkDeviceMemory baseTextureMemory{ VK_NULL_HANDLE };
	VkImageView baseTextureView{ VK_NULL_HANDLE };

	VkImage overlayTextureImage{ VK_NULL_HANDLE };
	VkDeviceMemory overlayTextureMemory{ VK_NULL_HANDLE };
	VkImageView overlayTextureView{ VK_NULL_HANDLE };
};