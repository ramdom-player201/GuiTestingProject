#pragma once

//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

// Forward declarations
class BaseWindow;
class GraphicsPipeline;
class ShaderModule;

// actual class
class Renderer {
public:
	Renderer(
		VkDevice logicalDevice,
		VkPhysicalDevice physicalDevice,
		VkQueue graphicsQueue,
		VkQueue presentQueue
	);
	~Renderer();

	void CreateRenderPass(VkFormat swapchainImageFormat);
	void CreatePipelines();
	void CreateFramebuffers(BaseWindow& window);
	void RecordCommandBuffers(BaseWindow& window);
	void RecreateSwapchain(BaseWindow& window);
	void Cleanup();
private:
	// cached handles
	VkDevice logicalDevice;
	VkPhysicalDevice physicalDevice;
	VkQueue graphcsQueue;
	VkQueue presentQueue;

	VkRenderPass renderPass{ VK_NULL_HANDLE };
	GraphicsPipeline* basicPipeline{ nullptr };
	VkCommandPool commandPool{ VK_NULL_HANDLE };

	void CreateCommandPool();
	void DestroyFramebuffers(BaseWindow& window);
};

// Own pipelines, shader modules, and render passes