#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <string_view>

class GuiRenderResources {
private:
	// Vulkan primitives
	VkInstance vkInstance{ VK_NULL_HANDLE };
	VkPhysicalDevice vkPhysicalDevice{ VK_NULL_HANDLE };
	VkDevice vkLogicalDevice{ VK_NULL_HANDLE };

	// Memory type lookup helper
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

	// Pipeline resources
	VkRenderPass pipelineRenderPass{ VK_NULL_HANDLE };
	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	VkPipeline pipelineShapes{ VK_NULL_HANDLE };
	VkPipeline pipelineText{ VK_NULL_HANDLE };
	VkPipeline pipelineTextures{ VK_NULL_HANDLE };

	// SSBO shared infrastructure (buffers are local to each GuiLayout's GuiClipSsbo for parallelisation reasons)
	VkDescriptorSetLayout ssboDescriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorPool ssboDescriptorPool{ VK_NULL_HANDLE };

	// Quad resources, static uint quad used in instanced shape rendering
	VkBuffer quadBuffer{ VK_NULL_HANDLE };
	VkDeviceMemory quadBufferMemory{ VK_NULL_HANDLE };

	// Texture sampler
	VkSampler textureSampler{ VK_NULL_HANDLE };

	// Initialisation flag
	bool initialised{ false };

	// Internal creation helpers
	void Create_RenderPass();
	void Create_Sampler();
	void Create_PipelineLayout();
	void Create_SsboDescriptorSetLayout();
	void Create_SsboDescriptorPool();
	void Create_QuadBuffer();
	VkPipeline Create_Pipeline(std::string_view vertPath, std::string_view fragPath);
	VkShaderModule Create_ShaderModule(std::string_view path);

public:
	GuiRenderResources();
	~GuiRenderResources();

	void Cleanup();

	// called once from VulkanHandler during setup
	void Initialise(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice logicalDevice);

	// Safety locks
	GuiRenderResources(const GuiRenderResources&) = delete;
	GuiRenderResources& operator=(const GuiRenderResources&) = delete;
	GuiRenderResources(GuiRenderResources&&) = delete;
	GuiRenderResources& operator=(GuiRenderResources&&) = delete;

	// Pipeline getters
	VkRenderPass GetRenderPass() const { return pipelineRenderPass; }
	VkPipelineLayout GetPipelineLayout() const { return pipelineLayout; }
	VkPipeline GetShapePipeline() const { return pipelineShapes; }
	VkPipeline GetTextPipeline() const { return pipelineText; }
	VkPipeline GetTexturePipeline() const { return pipelineTextures; }

	// SSBO getters
	VkDescriptorSetLayout GetSsboDescriptorSetLayout() const { return ssboDescriptorSetLayout; }
	VkDescriptorPool GetSsboDescriptorPool() const { return ssboDescriptorPool; }

	// SSBO descriptor set allocation (used by GuiClipSsbo)
	// Return VK_NULL_HANDLE if pool exhausted, logtype is ERROR and not CRITICAL
	VkDescriptorSet AllocateSsboDescriptorSet(VkBuffer ssboBuffer, VkDeviceSize ssboSize) const;
	void FreeSsboDescriptorSet(VkDescriptorSet set) const;

	// Quad buffer getter
	VkBuffer GetQuadBuffer() const { return quadBuffer; }

	// Texture sampler getter
	VkSampler GetTextureSampler() const { return textureSampler; }

	// Initialisation checker
	bool IsInitialised() const { return initialised; }

	// PushConstant layout for vertices
	struct PushConstantLayout {
		static constexpr uint32_t VERTEX_OFFSET{ 0 };
		static constexpr uint32_t VERTEX_SIZE{ 16 }; // width, height, offsetX, offsetY
	};

	static constexpr std::string_view className{ "GuiRenderResources" };
};