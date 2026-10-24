#pragma once

#include "LayoutTypes.h"
#include <vulkan/vulkan.h>
#include <cstdint>
#include <string_view>
#include <vector>

class VulkanHandler;
struct GuiBatches;
struct RenderTarget;

class GuiBatchRootRenderer {
private:
	VulkanHandler& vulkanHandler;

	struct InstanceBuffer {
		VkBuffer buffer{ VK_NULL_HANDLE };		 // Handle for the buffer
		VkDeviceMemory memory{ VK_NULL_HANDLE }; // Device-visible memory allocated on the GPU where this buffer lives
		void* mapped{ nullptr };				 // CPU-accessible pointer for direct CPU-to-GPU data writes
		VkDeviceSize capacity{ 0 };				 // Maximum bytes this buffer can hold
		VkDeviceSize inUse{ 0 };				 // Number of bytes actually used by this buffer
	};

	InstanceBuffer shapeInstances;	 // Holds shape instances
	InstanceBuffer textInstances;	 // Empty stub
	InstanceBuffer textureInstances; // Empty stub

	InstanceBuffer Create_InstanceBuffer(VkDeviceSize capacityBytes);
	void Destroy_InstanceBuffer(InstanceBuffer& buf);

	void Cleanup();

	// Persistenly allocated vectors for inverted rendering
	std::vector<DrawRequest> reversedRenderList;
	std::vector<ShapeInstance> reversedShapes;
	std::vector<TextInstance> reversedText;			// not yet used
	std::vector<TextureInstance> reversedTextures;	// not yet used

public:
	GuiBatchRootRenderer(VulkanHandler& vk);
	~GuiBatchRootRenderer();

	void CreateResources(uint32_t maxInstances = 8192); // This should be called only once, requires vk to be initialised

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
		const RenderTarget& target,
		VkDescriptorSet clipDescriptorSet
	);

	static constexpr std::string_view className{ "GuiBatchRootRenderer" };
};