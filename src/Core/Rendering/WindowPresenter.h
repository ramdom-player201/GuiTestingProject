#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <string>

#include "GuiLayout.h"

// Forward declarations
class VulkanHandler;
struct SwapchainData; // Defined in LayoutTypes.h

class WindowPresenter {
private:
	VulkanHandler& vulkanHandler;
	GuiLayout guiLayout;

	// State tracking
	VkExtent2D currentExtent{ 0,0 };
	VkFormat currentSwapchainFormat{ VK_FORMAT_UNDEFINED };
	bool bForceTextureBarrier{ true }; // Force state transition for GuiLayout's texture on first frame (init step)

	// Persistent resources (created once and only refreshed on format change)
	VkRenderPass renderPass{ VK_NULL_HANDLE };
	VkPipeline pipeline{ VK_NULL_HANDLE };
	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
	VkSampler textureSampler{ VK_NULL_HANDLE };
	VkCommandPool commandPool{ VK_NULL_HANDLE };

	// Transient Resources (refreshed on swapchain recreation / window resize)
	std::vector<VkFramebuffer> framebuffers;
	std::vector<VkCommandBuffer> commandBuffers;
	VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
	std::vector<VkDescriptorSet> descriptorSets;

	// Pipeline objects are persistent: created once and only changed in the unlikely event of a format change
	// Frame resources are transient: created at init, and refreshed on swapchain recreation caused by window resize

	// Cleanup Helpers
	void CleanupPersistentObjects(); // persistent elements, excluding commandPool
	void CleanupFrameResources(); // transient elements

	// Setup helpers
	void CreatePersistentObjects(VkFormat format); // generates persistent elements (commandPool only if not null)
	void CreateFrameResources(const SwapchainData& data); // generates transient elements

	// Additional setup sub-functions
	void Persistent_CreateRenderPass();
	void Persistent_CreateDescriptorResources();
	void Persistent_CreatePipeline();
	void Frame_AllocateCommandBuffers(size_t count);

	// Rendering helpers
	void DrawFullscreenQuad(VkCommandBuffer cmd, uint32_t imageIndex);
	void UpdateDescriptorSet(uint32_t imageIndex, VkImageView textureView);
	VkShaderModule CreateShaderModule(const std::string& filename);

public:
	WindowPresenter(VulkanHandler& vulkanHandler);
	~WindowPresenter();

	//void ProcessGui(const InputEvent& input); // Process input and update GUI
	//VkCommandBuffer Present(uint32_t imageIndex); // Records commands to draw the GUI to the swapchain
	void Refresh(const SwapchainData& data); // Handles Init, Resize, and Format changes

	VkCommandBuffer UpdateAndPresent(InputEvent& input, uint32_t imageIndex);

	// Safety locks
	WindowPresenter() = delete;
	WindowPresenter(const WindowPresenter&) = delete;
	WindowPresenter& operator=(const WindowPresenter&) = delete;
	WindowPresenter(WindowPresenter&&) = delete;
	WindowPresenter& operator=(WindowPresenter&&) = delete;

	static constexpr std::string_view className{ "WindowPresenter" };
};