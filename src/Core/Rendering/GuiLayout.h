#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <memory>

#include "LayoutTypes.h" // Required for DrawRect (possibly PanelType, but that is stub right now)
#include "GuiRenderResources.h"
#include "GuiBatchRootRenderer.h"
#include "GuiElements/UiFrame.h"

// Forward declarations
class VulkanHandler;

enum class PageMode :uint8_t {
	DockableEditor,
	DedicatedViewer,
	SplashScreen
};

class GuiLayout {
private:
	VulkanHandler& vulkanHandler;

	uint32_t currentWindowWidth{ 0 };
	uint32_t currentWindowHeight{ 0 };

	// Tree
	std::unique_ptr<UiFrame> treeRoot;

	// Batches
	GuiBatches batches;

	// Render resources
	GuiRenderResources renderResources;

	// Renderer
	GuiBatchRootRenderer renderer;

	// Render target (output texture)
	struct LayerTexture {
		VkImage image{ VK_NULL_HANDLE };
		VkDeviceMemory memory{ VK_NULL_HANDLE };
		VkImageView view{ VK_NULL_HANDLE };
		VkFramebuffer framebuffer{ VK_NULL_HANDLE };
	};
	LayerTexture layerTexture;

	// Cached map for viewport layout data (populated during CalculateLayout by finding ViewportWidgets)
	//std::unordered_map<uint32_t, DrawRect> viewportLayoutRequests;
	// ^ Deprecated as Viewports are no longer external

	const VkFormat FORMAT{ VK_FORMAT_R8G8B8A8_SRGB };

	// Texture management, as window may be resized
	void CreateLayerTexture();
	void CleanupLayerTexture();

public:
	explicit GuiLayout(VulkanHandler& vulkanHandler);
	~GuiLayout();

	void CreateRenderResources();

	// Safety locks
	GuiLayout() = delete;
	GuiLayout(const GuiLayout&) = delete;
	GuiLayout& operator=(const GuiLayout&) = delete;
	GuiLayout(GuiLayout&&) = delete;
	GuiLayout& operator=(GuiLayout&&) = delete;

	// Traverse UI tree to populate batches, etc
	InputEventResult ProcessGui(const InputEvent& event);

	// Records draw commands into provided command buffer
	void Render(VkCommandBuffer cmd);

	// Output
	VkImageView GetTextureView() const { return layerTexture.view; }
	const GuiRenderResources& GetRenderResources() const { return renderResources; }

	// Sizing
	void Resize(uint32_t width, uint32_t height);

	// Load gui tree based on app state
	void LoadPage(PageMode mode, const std::string& layoutFilePath); // WIP stub

	// ClassName
	static constexpr std::string_view className{ "GuiLayout" };
};