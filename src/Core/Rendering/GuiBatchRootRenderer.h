#pragma once

#include <vulkan/vulkan.h>
#include "string"
#include <cstdint>

class VulkanHandler;
class GuiRenderResources;
struct GuiBatches;
struct RenderTarget;

class GuiBatchRootRenderer {
private:
	VulkanHandler& vulkanHandler;
	const GuiRenderResources& resources;

	VkBuffer vertexBuffer{ VK_NULL_HANDLE };
	VkDeviceMemory vertexBufferMemory{ VK_NULL_HANDLE };
	void* mappedMemory{ nullptr };
	size_t bufferSize{ 0 };

	void CreateVertexBuffer(size_t size);
	void DestroyVertexBuffer();
	void EnsureBufferSize(size_t requiredSize);

	void UploadAndDraw(
		VkCommandBuffer cmd,
		VkPipeline pipeline,
		const void* data,
		size_t dataSize,
		uint32_t vertexCount
	);

	void Cleanup();

public:
	GuiBatchRootRenderer(VulkanHandler& vk, const GuiRenderResources& resources);
	~GuiBatchRootRenderer();

	void CreateResources(uint32_t maxVertices = 8192);

	// Safety locks
	GuiBatchRootRenderer() = delete;
	GuiBatchRootRenderer(const GuiBatchRootRenderer&) = delete;
	GuiBatchRootRenderer& operator=(const GuiBatchRootRenderer&) = delete;
	GuiBatchRootRenderer(GuiBatchRootRenderer&&) = delete;
	GuiBatchRootRenderer& operator=(GuiBatchRootRenderer&&) = delete;

	// Records commands to render all batches to the target texture.
	// Assumes cmd is NOT inside a render pass.
	void Render(
		VkCommandBuffer cmd,
		const GuiBatches& batches,
		const RenderTarget& target
	);

	static constexpr std::string_view className{ "GuiBatchRootRenderer" };
};