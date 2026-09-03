#pragma once

#include <vulkan/vulkan.h>
#include <string>

class VulkanHandler;

class GuiRenderResources {
private:
	VulkanHandler& vulkanHandler;

	VkRenderPass renderPass{ VK_NULL_HANDLE };
	VkPipeline shapePipeline{ VK_NULL_HANDLE };
	VkPipeline textPipeline{ VK_NULL_HANDLE };
	VkPipeline texturePipeline{ VK_NULL_HANDLE };
	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	VkSampler textureSampler{ VK_NULL_HANDLE };

	VkShaderModule CreateShaderModule(std::string_view path);
	VkPipeline CreatePipeline(std::string_view vertPath, std::string_view fragPath, bool hasSampler);

	void CreateRenderPass();
	void CreateSampler();
	void CreatePipelineLayout();

	void Cleanup();
public:
	GuiRenderResources(VulkanHandler& vk);
	~GuiRenderResources();

	void CreateResources();

	// Safety locks
	GuiRenderResources() = delete;
	GuiRenderResources(const GuiRenderResources&) = delete;
	GuiRenderResources& operator=(const GuiRenderResources&) = delete;
	GuiRenderResources(GuiRenderResources&&) = delete;
	GuiRenderResources& operator=(GuiRenderResources&&) = delete;

	VkRenderPass GetRenderPass() const { return renderPass; }
	VkPipeline GetShapePipeline() const { return shapePipeline; }
	VkPipeline GetTextPipeline() const { return textPipeline; }
	VkPipeline GetTexturePipeline() const { return texturePipeline; }
	VkPipelineLayout GetPipelineLayout() const { return pipelineLayout; }
	VkSampler GetTextureSampler() const { return textureSampler; }

	static constexpr std::string_view className{ "GuiRenderResources" };
};