// LayoutCompositor holds rendering architecture for the gui

// gui is built from a gui compiler (GuiLayout) into an image
// gui is rebuilt any time it changes, otherwise the existing copy is used
// gui defines the placement of viewport subframes, but does not manage them

// gui -> offscreen image
// 3D -> offscreen image, account for size of sub-cut of screen
// draw gui + 3D viewports + overlays

// LayoutCompositor <- window level generalist and gui handler
// GuiLayout <- 2D gui renderer (base layer + overlay)
// Viewport <- 3D sub-renderer

#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <map>
#include <string>

#include "Viewport.h"
#include "GuiLayout.h"

#include "LayoutTypes.h"

// Forward declarations
class VulkanHandler;
class InputEvent;


// Represents a single texture to be drawn to the screen
struct CompositingLayer {
	VkImageView textureView{ VK_NULL_HANDLE };
	DrawRect screenRect{};
};

class LayoutCompositor {
public:
	LayoutCompositor(VulkanHandler& vulkanHandler);
	~LayoutCompositor();

	// Lifecycle (managed by BaseWindow)
	void CreateCompositeResources(VkFormat swapchainFormat);
	void CreateFramebuffers(const std::vector<VkImageView>& swapchainImageViews, VkExtent2D swapchainExtent);

	void CleanupCompositeResources();
	void CleanupFramebuffers();

	// Primary
	VkCommandBuffer RecordCommands(uint32_t imageIndex);
	void PassInputs(const InputEvent& event);

	// Safety locks
	LayoutCompositor() = delete;
	LayoutCompositor(const LayoutCompositor&) = delete;
	LayoutCompositor& operator=(const LayoutCompositor&) = delete;
	LayoutCompositor(LayoutCompositor&&) = delete;
	LayoutCompositor& operator=(LayoutCompositor&&) = delete;

	// ClassName
	static constexpr std::string_view className{ "LayoutCompositor" };
private:
	// References
	VulkanHandler& vulkanHandler; // access VkDevice, VkQueues, etc

	// Subsystems
	GuiLayout guiLayout; // The 2D gui system (base layer and overlay)
	std::map<uint32_t, std::unique_ptr<Viewport>> viewports; // 3D sub-renderers

	// Frame-specific state (cleared and rebuilt every frame during GatherCompositingLayers)
	std::vector<CompositingLayer> activeLayersThisFrame;

	// Command buffering (owned by compositor but acted on by BaseWindow)
	VkCommandPool commandPool{ VK_NULL_HANDLE };
	std::vector<VkCommandBuffer> commandBuffers;

	// Framebuffer targets (regenerated dynamically on swapchain resize)
	std::vector<VkFramebuffer> framebuffers;
	VkExtent2D currentExtent{ 0,0 };
	VkFormat currentSwapchainFormat{ VK_FORMAT_UNDEFINED };

	// Compositing resources
	VkRenderPass compositeRenderPass{ VK_NULL_HANDLE };
	VkPipeline compositePipeline{ VK_NULL_HANDLE };
	VkPipelineLayout compositePipelineLayout{ VK_NULL_HANDLE };

	// Descriptor resources
	VkDescriptorSetLayout compositeDescriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorPool compositeDescriptorPool{ VK_NULL_HANDLE };
	std::vector<VkDescriptorSet> compositeDescriptorSets;

	// Single quad vertex buffer to composite and draw textures to the window
	VkBuffer quadVertexBuffer{ VK_NULL_HANDLE };
	VkSampler compositeSampler{ VK_NULL_HANDLE };
	VkDeviceMemory quadVertexBufferMemory{ VK_NULL_HANDLE };

	// Helper functions
	void CreateCommandPool();
	void AllocateCommandBuffers(size_t count);
	void CreateQuadVertexBuffer();
	void CreateRenderPass();
	void CreateDescriptorResources();
	void CreatePipeline();

	// Subsystem management
	void GatherCompositingLayers();
	void SyncViewports(const std::unordered_map<uint32_t, DrawRect>& requestedRects);

	// Vulkan helper
	void UpdateDescriptorSet(uint32_t imageIndex, VkImageView textureView);

	// Handle shader files
	VkShaderModule CreateShaderModule(const std::string& filename);
};
















// TEMPORARY AI GENERATED TEMPLATE: USE AS REFERENCE:
/*
#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

class VulkanHandler;
class GraphicsPipeline;

class Viewport {
public:
	Viewport(VulkanHandler& handler, uint32_t initialWidth, uint32_t initialHeight);
	~Viewport();

	// Called by Renderer when the GUI panel is resized
	void resize(uint32_t newWidth, uint32_t newHeight);

	// Called by Renderer during the main render pass
	// Renders the 3D scene to the offscreen target
	void render3D(VkCommandBuffer cmd, uint32_t currentFrameIndex);

	// Getter used by the GUI to draw this viewport as a textured quad
	VkImageView getColorTextureView() const { return offscreenColorView; }
	VkExtent2D getExtent() const { return extent; }

private:
	VulkanHandler& vulkanHandler;
	VkExtent2D extent;

	// --- OFFSCREEN RENDER TARGETS (Not the Swapchain!) ---
	VkImage offscreenColorImage;
	VkDeviceMemory offscreenColorMemory;
	VkImageView offscreenColorView;

	VkImage offscreenDepthImage;
	VkDeviceMemory offscreenDepthMemory;
	VkImageView offscreenDepthView;

	VkFramebuffer offscreenFramebuffer;
	VkRenderPass offscreenRenderPass; // Requires Depth + Color

	// --- CAD SPECIFIC 3D PIPELINES ---
	std::unique_ptr<GraphicsPipeline> solidMeshPipeline;
	std::unique_ptr<GraphicsPipeline> wireframePipeline;
	std::unique_ptr<GraphicsPipeline> markerPipeline;
	std::unique_ptr<GraphicsPipeline> outlinePipeline;

	// --- 3D RESOURCE BINDING ---
	VkDescriptorSetLayout cameraDescriptorSetLayout;
	VkDescriptorSetLayout objectDescriptorSetLayout;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> cameraDescriptorSets; // Per frame in flight

	// --- HELPERS ---
	void createOffscreenResources();
	void createPipelines();
	void cleanupOffscreenResources(); // Needed for resizing
};
*/