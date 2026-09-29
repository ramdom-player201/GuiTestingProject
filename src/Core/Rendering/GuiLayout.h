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
	std::unique_ptr<UiFrame> treeRoot{ nullptr };

	// Batches
	GuiBatches batches;

	// Render resources and root renderer
	GuiRenderResources renderResources;
	GuiBatchRootRenderer renderer;

	// Render target (output texture)
	struct LayerTexture {
		VkImage image{ VK_NULL_HANDLE };
		VkDeviceMemory memory{ VK_NULL_HANDLE };
		VkImageView view{ VK_NULL_HANDLE };
		VkFramebuffer framebuffer{ VK_NULL_HANDLE };
	};
	LayerTexture layerTexture; // <- do we want to keep this a struct as opposed to just raw params?

	// Texture management, as window may be resized
	void CreateLayerTexture();
	void CleanupLayerTexture();

public:
	explicit GuiLayout(VulkanHandler& vulkanHandler);
	~GuiLayout();

	void InitialiseRenderResources(); // Called once only, requires vk to be initialised, initialises GuiRenderResources

	// Safety locks
	GuiLayout() = delete;
	GuiLayout(const GuiLayout&) = delete;
	GuiLayout& operator=(const GuiLayout&) = delete;
	GuiLayout(GuiLayout&&) = delete;
	GuiLayout& operator=(GuiLayout&&) = delete;

	// Traverse tree, populate batches and record draw commands
	bool UpdateAndRender(InputEvent& event, VkCommandBuffer cmd);
	// The tree is traversed depth-first.
	// A batch ref travels down the tree.
	// UI subelements (UiScrollingFrame, etc) can overload batches with their own.
	// These elements record their render commands during traversal, when exiting their nodes upwards.
	// The top-level batches owned by GuiLayout are handled after tree traversal.

	// Output
	VkImageView GetTextureView() const { return layerTexture.view; }
	VkImage GetTextureImage() const { return layerTexture.image; }
	const GuiRenderResources& GetRenderResources() const { return renderResources; }

	// Sizing
	void Refresh(uint32_t width, uint32_t height);

	// Load gui tree based on app state
	void LoadPage(PageMode mode, const std::string& layoutFilePath); // WIP stub

	// ClassName
	static constexpr std::string_view className{ "GuiLayout" };
};