#pragma once

#include "../Services/LogService.h"

#include <vulkan/vulkan.h>
#include <vector>

class VulkanHandler;
struct ClipRect;

class GuiClipSsbo {
private:
	VulkanHandler& vulkanHandler;

	// SSBO buffer (holds ClipRect data, shared across all renderers in this GuiLayout)
	VkBuffer ssboBuffer{ VK_NULL_HANDLE };
	VkDeviceMemory ssboBufferMemory{ VK_NULL_HANDLE };
	void* ssboMapped{ nullptr };
	VkDeviceSize ssboCapacity{ 0 };        // Max bytes the buffer can hold
	VkDeviceSize ssboCurrentSize{ 0 };     // Bytes actually used this frame

	// Descriptor set pointing to ssboBuffer
	VkDescriptorSet ssboDescriptorSet{ VK_NULL_HANDLE };

	// Debug debounce to prevent error spam
	bool capacityOverflowErrorDebounce{ false };

	void Create_Buffer(VkDeviceSize capacityBytes);
	void Destroy_Buffer();

public:
	GuiClipSsbo(VulkanHandler& vk);
	~GuiClipSsbo();

	// Called once during GuiLayout initialisation
	void CreateResources(uint32_t maxClipRects = 1024);

	// Called every frame to upload the accumulated clipchain to the SSBO
	// Returns false if the clipchain exceeds capacity
	bool Upload(const std::vector<ClipRect>& clipChain);
	// Do we want dynamic resizing/reallocation on exceed?
	// ^ This would have to occur for the next frame I suppose
	// Not a priority right now unless simple to implement or already handled

	// For binding during a render pass
	VkDescriptorSet GetDescriptorSet() const { return ssboDescriptorSet; }

	// Safety locks
	GuiClipSsbo() = delete;
	GuiClipSsbo(const GuiClipSsbo&) = delete;
	GuiClipSsbo& operator=(const GuiClipSsbo&) = delete;
	GuiClipSsbo(GuiClipSsbo&&) = delete;
	GuiClipSsbo& operator=(GuiClipSsbo&&) = delete;

	static constexpr std::string_view className{ "GuiClipSsbo" };
};