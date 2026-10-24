#include "GuiClipSsbo.h"

#include "LayoutTypes.h"
#include "../Services/VulkanHandler.h"

#include <cstring> // std::memcpy

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

GuiClipSsbo::GuiClipSsbo(VulkanHandler& vk) :vulkanHandler(vk) {
	LogService::Log(LogType::TRACE, className, "Constructor", "Created GuiClipSsbo");
}

GuiClipSsbo::~GuiClipSsbo() {
	Destroy_Buffer();

	// Return descriptor set to the pool
	if (ssboDescriptorSet != VK_NULL_HANDLE) {
		vulkanHandler.GetGuiRenderResources().FreeSsboDescriptorSet(ssboDescriptorSet);
		ssboDescriptorSet = VK_NULL_HANDLE;
	}

	LogService::Log(LogType::TRACE, className, "Destructor", "Destroyed GuiClipSsbo");
}

void GuiClipSsbo::CreateResources(uint32_t maxClipRects) {
	constexpr std::string_view functionName{ "CreateResources" };

	VkDeviceSize capacityBytes{ static_cast<VkDeviceSize>(maxClipRects) * sizeof(ClipRect) };
	Create_Buffer(capacityBytes);

	// Allocate descriptor set for SSBO buffer
	const GuiRenderResources& resources{ vulkanHandler.GetGuiRenderResources() };
	ssboDescriptorSet = resources.AllocateSsboDescriptorSet(ssboBuffer, capacityBytes);

	if (ssboDescriptorSet == VK_NULL_HANDLE) {
		// Descriptor pool too full to allocate another SSBO
		LogService::Log(
			LogType::ERROR, className, functionName,
			"Failed to allocate descriptor set for clipping SSBO, how many windows and/or tabs do you have open?"
		);
		return; // Early exit
	}

	LogService::Log(
		LogType::SUCCESS, className, functionName,
		"GuiClipSsbo created successfully :: allocated capacity for [" +
		std::to_string(maxClipRects) + "] clip rects, at [" +
		std::to_string(capacityBytes) + "] bytes"
	);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// BUFFER MANAGEMENT
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiClipSsbo::Create_Buffer(VkDeviceSize capacityBytes) {
	constexpr std::string_view functionName{ "Create_Buffer" };

	VkDevice device{ vulkanHandler.GetLogicalDevice() };

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = capacityBytes;
	bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &ssboBuffer) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create SSBO buffer");
		throw std::runtime_error("Failed to create SSBO buffer");
		// ^ could be caused by VK_ERROR_OUT_OF_DEVICE_MEMORY
		// There's a lot of runtime error throws throughout the program
		// It'd be a good idea to go through all of them and identify the specific error states VkResult can return
		// These logs might be seen by standard users, so it'd be a good idea to include actually useful information in errors
		// An error may be unrecoverable, but they may still be avoidable in the way the user uses the program
		// EG, if the user knows that the crash was specifically due to GPU memory being exceeded, they can change their behaviour to stay in limits
		// Really though, the program should be designed in such a way that the user will know that they are approaching limits way before they are hit
		// This is something to consider, but not a core priority at the current time
	}

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device, ssboBuffer, &memReqs);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = vulkanHandler.FindMemoryType(
		memReqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &ssboBufferMemory) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to allocate SSBO memory");
		throw std::runtime_error("Failed to allocate SSBO memory");
	}

	vkBindBufferMemory(device, ssboBuffer, ssboBufferMemory, 0);

	// Persistently map for the lifetime of the buffer
	vkMapMemory(device, ssboBufferMemory, 0, capacityBytes, 0, &ssboMapped);

	ssboCapacity = capacityBytes;
}

void GuiClipSsbo::Destroy_Buffer() {
	constexpr std::string_view functionName{ "Destroy_Buffer" };

	VkDevice device{ vulkanHandler.GetLogicalDevice() };

	if (ssboMapped != nullptr) {
		vkUnmapMemory(device, ssboBufferMemory);
		ssboMapped = nullptr;
	}
	if (ssboBuffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device, ssboBuffer, nullptr);
		ssboBuffer = VK_NULL_HANDLE;
	}
	if (ssboBufferMemory != VK_NULL_HANDLE) {
		vkFreeMemory(device, ssboBufferMemory, nullptr);
		ssboBufferMemory = VK_NULL_HANDLE;
	}

	ssboCapacity = 0;
	ssboCurrentSize = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// PER-FRAME UPLOAD
//////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GuiClipSsbo::Upload(const std::vector<ClipRect>& clipChain) {
	constexpr std::string_view functionName{ "Upload" };

	// No descriptor set means SSBO is unusable (allocation failed during CreateResources)
	if (ssboDescriptorSet == VK_NULL_HANDLE || ssboMapped == nullptr) {
		return false; // skip SSBO upload with false for upload error
	}

	VkDeviceSize requiredBytes{ static_cast<VkDeviceSize>(clipChain.size()) * sizeof(ClipRect) };

	if (requiredBytes > ssboCapacity) {
		// Capacity exceeded, clamp to limit and log error
		if (!capacityOverflowErrorDebounce) {
			uint32_t maxClipRects{ static_cast<uint32_t>(ssboCapacity / sizeof(ClipRect)) };
			LogService::Log(
				LogType::ERROR, className, functionName,
				"Clip chain size <" + std::to_string(clipChain.size()) +
				"> ClipRects exceeds SSBO capacity <" + std::to_string(maxClipRects) +
				"> ClipRects. Clipping data past this point shall be discarded"
			);
			capacityOverflowErrorDebounce = true; // debounce to prevent error spam every frame
		}
		ssboCurrentSize = ssboCapacity;
		std::memcpy(ssboMapped, clipChain.data(), ssboCurrentSize);
		return false; // partial upload with false for upload error
	}

	capacityOverflowErrorDebounce = false; // clear debounce

	ssboCurrentSize = requiredBytes;
	std::memcpy(ssboMapped, clipChain.data(), ssboCurrentSize);
	return true; // successful upload with true for no errors
}