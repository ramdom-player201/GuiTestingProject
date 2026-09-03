#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <string>

#include "GuiLayout.h"

class VulkanHandler;

class LayoutCompositor {
private:
	VulkanHandler& vulkanHandler;

	GuiLayout guiLayout;

	// Framebuffer targets
	std::vector<VkFramebuffer> framebuffers;
	VkExtent2D currentExtent{ 0, 0 };
	VkFormat currentSwapchainFormat{ VK_FORMAT_UNDEFINED };

	// Pipeline resources
	VkRenderPass renderPass{ VK_NULL_HANDLE };
	VkPipeline pipeline{ VK_NULL_HANDLE };
	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };

	// Descriptor resources
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
	std::vector<VkDescriptorSet> descriptorSets;

	// Vertex buffer for fullscreen quad
	VkBuffer quadVertexBuffer{ VK_NULL_HANDLE };
	VkDeviceMemory quadVertexBufferMemory{ VK_NULL_HANDLE };
	VkSampler textureSampler{ VK_NULL_HANDLE };

	// Command buffers
	VkCommandPool commandPool{ VK_NULL_HANDLE };
	std::vector<VkCommandBuffer> commandBuffers;

	// Helper initialisations
	void CreateCommandPool();
	void AllocateCommandBuffers(size_t count);
	void CreateQuadVertexBuffer();
	void CreateRenderPass(); 
	void CreateDescriptorResources();
	void CreatePipeline();

	void UpdateDescriptorSet(uint32_t imageIndex, VkImageView textureView);
	VkShaderModule CreateShaderModule(const std::string& filename);
public:
	LayoutCompositor(VulkanHandler& vulkanHandler);
	~LayoutCompositor();

	// Lifecycle (called from BaseWindow)
	void CreateResources(VkFormat swapchainFormat);
	void CreateFramebuffers(const std::vector<VkImageView>& swapchainImageViews, VkExtent2D swapchainExtent);
	void InitialiseGui(uint32_t width, uint32_t height);

	void CleanupResources();
	void CleanupFramebuffers();

    // Update loop
    void ProcessGui(const InputEvent& input);
    VkCommandBuffer RecordCommands(uint32_t imageIndex);

	// Safety locks
	LayoutCompositor() = delete;
	LayoutCompositor(const LayoutCompositor&) = delete;
	LayoutCompositor& operator=(const LayoutCompositor&) = delete;
	LayoutCompositor(LayoutCompositor&&) = delete;
	LayoutCompositor& operator=(LayoutCompositor&&) = delete;

	static constexpr std::string_view className{ "LayoutCompositor" };
};