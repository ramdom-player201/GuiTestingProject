#include "GuiBatchRootRenderer.h"

#include "LayoutTypes.h"
#include "GuiRenderResources.h"
#include "../Services/VulkanHandler.h"
#include "../Services/LogService.h"

#include <cstring>

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANTS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {
	constexpr uint32_t VERTEX_PUSH_OFFSET{ 0 };
	constexpr uint32_t FRAGMENT_PUSH_OFFSET{ 16 };

	// Must match shader layout exactly
	struct ClipPushData {
		ClipRect clips[3]{};
		uint32_t clipCount{ 0 };
		uint32_t padding[3]{};
	};
	static_assert(sizeof(ClipPushData) == 112, "ClipPushData size mismatch");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

GuiBatchRootRenderer::GuiBatchRootRenderer(
	VulkanHandler& vk,
	const GuiRenderResources& resources
) : vulkanHandler(vk), resources(resources) {
}

GuiBatchRootRenderer::~GuiBatchRootRenderer() {
	Cleanup();
}

void GuiBatchRootRenderer::CreateResources(uint32_t maxVertices) {
	Cleanup();

	// Assume ShapeVertex is the largest vertex format
	CreateVertexBuffer(static_cast<size_t>(maxVertices) * sizeof(ShapeVertex));
}

void GuiBatchRootRenderer::Cleanup() {
	constexpr std::string_view functionName{ "Cleanup" };

	VkDevice device{ vulkanHandler.GetLogicalDevice() };
	if (device == VK_NULL_HANDLE) { return; }

	vkDeviceWaitIdle(device);
	DestroyVertexBuffer();

	LogService::Log(LogType::TRACE, className, functionName, "Cleaned GuiI nBatchRootRenderer");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// BUFFER MANAGEMENT
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiBatchRootRenderer::CreateVertexBuffer(size_t size) {
	constexpr std::string_view functionName{ "CreateVertexBuffer" };

	VkDevice device{ vulkanHandler.GetLogicalDevice() };

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create vertex buffer");
		throw std::runtime_error("Failed to create vertex buffer");
	}

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device, vertexBuffer, &memReqs);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = vulkanHandler.FindMemoryType(
		memReqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to allocate vertex buffer memory");
		throw std::runtime_error("Failed to allocate vertex buffer memory");
	}

	vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);
	vkMapMemory(device, vertexBufferMemory, 0, size, 0, &mappedMemory);

	bufferSize = size;
}

void GuiBatchRootRenderer::DestroyVertexBuffer() {
	VkDevice device{ vulkanHandler.GetLogicalDevice() };

	if (mappedMemory != nullptr) {
		vkUnmapMemory(device, vertexBufferMemory);
		mappedMemory = nullptr;
	}
	if (vertexBuffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vertexBuffer = VK_NULL_HANDLE;
	}
	if (vertexBufferMemory != VK_NULL_HANDLE) {
		vkFreeMemory(device, vertexBufferMemory, nullptr);
		vertexBufferMemory = VK_NULL_HANDLE;
	}
	bufferSize = 0;
}

void GuiBatchRootRenderer::EnsureBufferSize(size_t requiredSize) {
	if (requiredSize <= bufferSize) { return; } // Early exit if already sufficient

	// Double until it fits
	size_t newSize{ bufferSize };
	while (newSize < requiredSize) {
		newSize *= 2;
	}

	LogService::Log(LogType::TRACE, className, "EnsureBufferSize",
		"Growing vertex buffer from " + std::to_string(bufferSize) +
		" to " + std::to_string(newSize) + " bytes"
	);

	VkDevice device{ vulkanHandler.GetLogicalDevice() };
	vkDeviceWaitIdle(device);

	DestroyVertexBuffer();
	CreateVertexBuffer(newSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// RENDERING
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiBatchRootRenderer::Render(
	VkCommandBuffer cmd,
	const GuiBatches& batches,
	const RenderTarget& target
) {
	constexpr std::string_view functionName{ "Render" };

	if (target.framebuffer == VK_NULL_HANDLE) { return; }

	// Begin render pass
	VkRenderPassBeginInfo rpInfo{};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.renderPass = resources.GetRenderPass();
	rpInfo.framebuffer = target.framebuffer;
	rpInfo.renderArea.offset = { 0,0 };
	rpInfo.renderArea.extent = { target.width,target.height };

	VkClearValue clearValue = { {{0.0f, 0.0f, 0.0f, 0.0f}} };
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Dynamic viewport and scissor
	VkViewport viewport{
		0.0f,0.0f,
		static_cast<float>(target.width),
		static_cast<float>(target.height),
		0.0f,1.0f
	};
	VkRect2D scissor{ {0,0},{target.width,target.height} };

	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	// Push vertex-stage constants (viewport transform) <- constant for whole render
	float vertPush[4]{
		static_cast<float>(target.width),
		static_cast<float>(target.height),
		0.0f,
		0.0f
	};
	vkCmdPushConstants(
		cmd,
		resources.GetPipelineLayout(),
		VK_SHADER_STAGE_VERTEX_BIT,
		VERTEX_PUSH_OFFSET,
		sizeof(vertPush),
		vertPush
	);

	// Iterate Z buckets in back-to-front order
	if (!batches.IsEmpty()) {
		for (const auto& [z, bucket] : batches.zBuckets) {
			// Push fragment-stage constants (clip stack)
			// Always push full struct to avoid stale data from previous bucket
			ClipPushData clipData{};
			clipData.clipCount = bucket.clipStack.Count();
			for (uint32_t i{ 0 }; i < clipData.clipCount && i < 3; ++i) {
				clipData.clips[i] = bucket.clipStack.rects[i];
			}
			vkCmdPushConstants(
				cmd,
				resources.GetPipelineLayout(),
				VK_SHADER_STAGE_FRAGMENT_BIT,
				FRAGMENT_PUSH_OFFSET,
				sizeof(ClipPushData),
				&clipData
			);

			// Draw shapes
			if (!bucket.shapes.empty()) {
				UploadAndDraw(
					cmd,
					resources.GetShapePipeline(),
					bucket.shapes.data(),
					bucket.shapes.size() * sizeof(ShapeVertex),
					static_cast<uint32_t>(bucket.shapes.size())
				);
			}

			// Draw textures
			if (!bucket.textures.empty()) {
				LogService::Log(LogType::WIP, className, functionName, "Cannot draw textures because logic not yet implemented");
			}

			// Draw text
			if (!bucket.text.empty()) {
				LogService::Log(LogType::WIP, className, functionName, "Cannot draw text because logic not yet implemented");
			}

		}
	}

	vkCmdEndRenderPass(cmd);
}

void GuiBatchRootRenderer::UploadAndDraw(
	VkCommandBuffer cmd,
	VkPipeline pipeline,
	const void* data,
	size_t dataSize,
	uint32_t vertexCount
) {
	if (vertexCount == 0) { return; }

	EnsureBufferSize(dataSize);
	std::memcpy(mappedMemory, data, dataSize);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	VkDeviceSize offset{ 0 };
	vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer, &offset);

	vkCmdDraw(cmd, vertexCount, 1, 0, 0);
}