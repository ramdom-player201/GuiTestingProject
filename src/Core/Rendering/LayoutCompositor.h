// LayoutCompositor holds rendering architecture for the gui

// gui is built from a gui compiler into an image
// gui is rebuilt any time it changes, otherwise the existing copy is used
// gui contains 3D viewports as optional subframes

// gui -> offscreen image
// 3D -> offscreen image, account for size of sub-cut of screen
// draw gui + 3D viewports + overlays

// LayoutCompositor <- window level generalist and gui handler
// Viewport <- 3D sub-renderer

#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <map>

#include "Viewport.h"
#include "GuiLayout.h"

struct ViewportLayoutRect {
	uint32_t panelId;
	int32_t x;
	int32_t y;
	uint32_t width;
	uint32_t height;
};

// Forward declarations
class VulkanHandler;
class BaseWindow;
class Viewport;
class InputEvent;

class LayoutCompositor {
public:
	LayoutCompositor(BaseWindow& parentWindow, VulkanHandler& vulkanHandler);
	~LayoutCompositor();

	// Lifecycle
	void init();
	void handleWindowResize();

	// Primary
	void drawFrame();
	void passInputs(const InputEvent& event);

	// safety locks
	LayoutCompositor() = delete;
	LayoutCompositor(const LayoutCompositor&) = delete;
	LayoutCompositor& operator=(const LayoutCompositor&) = delete;
	LayoutCompositor(LayoutCompositor&&) = delete;
	LayoutCompositor& operator=(LayoutCompositor&&) = delete;
private:
	// References
	BaseWindow& parentWindow; // access SwapChainData, GLFWWindow, VkSurface through this
	VulkanHandler& vulkanHandler; // access VkDevice, VkQueues, etc

	// Subsystems
	GuiLayout guiLayout; // The 2D gui system
	std::map<uint32_t, std::unique_ptr<Viewport>> viewports; // 3D sub-renderers

	// Frame synchronisation
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	size_t currentFrame{ 0 };

	// Command buffering
	VkCommandPool commandPool{ VK_NULL_HANDLE };
	std::vector<VkCommandBuffer> commandBuffers;

	// Compositing resources
	VkRenderPass compositeRenderPass{ VK_NULL_HANDLE };
	VkPipeline compositePipeline{ VK_NULL_HANDLE };
	VkPipelineLayout compositePipelineLayout{ VK_NULL_HANDLE };

	VkDescriptorSetLayout compositeDescriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorPool compositeDescriptorPool{ VK_NULL_HANDLE };
	std::vector<VkDescriptorSet> compositeDescriptorSets;

	// Single quad vertex buffer to composite and draw textures to the window
	VkBuffer quadVertexBuffer{ VK_NULL_HANDLE };
	VkDeviceMemory quadVertexBufferMemory{ VK_NULL_HANDLE };

	// Helper functions
	void createSynchronisationObjects();
	void createCommandPoolAndBuffers();
	void createCompositeResources();
	void cleanupCompositeResources();

	// Compare GUI requested viewports against actually instantiated viewports
	void syncViewports(const std::vector<ViewportLayoutRect>& requestedRects);

	// Records the final draw commands to the swapchain
	void recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex);
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