#include "GuiBatchRootRenderer.h"

#include "GuiRenderResources.h"
#include "../Services/VulkanHandler.h"
#include "../Services/LogService.h"

#include <cstring>	 // std::memcpy
#include <stdexcept> // std::runtime_error
#include <algorithm> // std::reverse_copy

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANTS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {
	// Push constant layout
	constexpr uint32_t VERTEX_PUSH_OFFSET{ GuiRenderResources::PushConstantLayout::VERTEX_OFFSET };
	constexpr uint32_t VERTEX_PUSH_SIZE{ GuiRenderResources::PushConstantLayout::VERTEX_SIZE }; // width, height, offsetX, offsetY
	// Might want to move these to LayoutTypes (same for GuiRenderResources)

	// Triangle strip quad
	constexpr uint32_t QUAD_VERTEX_COUNT{ 4 };

	// Index 0 for quad, index 1 for instance
	constexpr uint32_t VERTEX_BINDING_QUAD{ 0 };
	constexpr uint32_t VERTEX_BINDING_INSTANCE{ 1 };

	// First descriptor set index for clipping buffer
	constexpr uint32_t DESCRIPTOR_SET_CLIP{ 0 };
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

GuiBatchRootRenderer::GuiBatchRootRenderer(VulkanHandler& vk) :vulkanHandler(vk) {
	LogService::Log(LogType::TRACE, className, "Constructor", "Created GuiBatchRootRenderer");
}

GuiBatchRootRenderer::~GuiBatchRootRenderer() {
	Cleanup();
	LogService::Log(LogType::TRACE, className, "Destructor", "Destroyed GuiBatchRootRenderer");
}

void GuiBatchRootRenderer::CreateResources(uint32_t maxInstances) {
	// Do we want to modify this to accept a struct of limits, so we can require different ones
	constexpr std::string_view functionName{ "CreateResources" };

	Cleanup();

	VkDeviceSize shapeCapacityBytes{ static_cast<VkDeviceSize>(maxInstances) * sizeof(ShapeInstance) };
	shapeInstances = Create_InstanceBuffer(shapeCapacityBytes);

	//textInstances; // remains null
	//textureInstances; // remains null

	LogService::Log(
		LogType::SUCCESS, className, functionName,
		"GuiBatchRootRenderer created with capacity for <" +
		std::to_string(maxInstances) + "> shape instances at <" +
		std::to_string(shapeCapacityBytes) + "> bytes"
	);
	LogService::Log(LogType::WIP, className, functionName, "Text and Texture instance resources not yet allocated");
}

void GuiBatchRootRenderer::Cleanup() {
	VkDevice device{ vulkanHandler.GetLogicalDevice() };
	if (device == VK_NULL_HANDLE) { return; }

	vkDeviceWaitIdle(device);

	Destroy_InstanceBuffer(shapeInstances);
	Destroy_InstanceBuffer(textInstances);		// Not yet created, so no-op result
	Destroy_InstanceBuffer(textureInstances);	// Not yet created, so no-op result
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// BUFFER MANAGEMENT
//////////////////////////////////////////////////////////////////////////////////////////////////////////


GuiBatchRootRenderer::InstanceBuffer GuiBatchRootRenderer::Create_InstanceBuffer(VkDeviceSize capacityBytes) {
	constexpr std::string_view functionName{ "Create_InstanceBuffer" };

	VkDevice device{ vulkanHandler.GetLogicalDevice() };

	// Setup buffer
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = capacityBytes;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer buffer{ VK_NULL_HANDLE };
	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create instance buffer");
		throw std::runtime_error("Failed to create instance buffer");
	}

	// Setup memory
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device, buffer, &memReqs);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = vulkanHandler.FindMemoryType(
		memReqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	VkDeviceMemory memory{ VK_NULL_HANDLE };
	if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
		vkDestroyBuffer(device, buffer, nullptr);
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to allocate instance buffer memory");
		throw std::runtime_error("Failed to allocate instance buffer memory");
	}

	// Map memory
	vkBindBufferMemory(device, buffer, memory, 0);

	void* mapped{ nullptr };
	vkMapMemory(device, memory, 0, capacityBytes, 0, &mapped);

	// Compile resulting InstanceBuffer
	InstanceBuffer result;
	result.buffer = buffer;
	result.memory = memory;
	result.mapped = mapped;
	result.capacity = capacityBytes;
	result.inUse = 0;
	return result;
}

void GuiBatchRootRenderer::Destroy_InstanceBuffer(InstanceBuffer& buf) {
	VkDevice device{ vulkanHandler.GetLogicalDevice() };

	if (buf.mapped != nullptr) {
		vkUnmapMemory(device, buf.memory);
		buf.mapped = nullptr;
	}
	if (buf.buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device, buf.buffer, nullptr);
		buf.buffer = VK_NULL_HANDLE;
	}
	if (buf.memory != VK_NULL_HANDLE) {
		vkFreeMemory(device, buf.memory, nullptr);
		buf.memory = VK_NULL_HANDLE;
	}

	buf.capacity = 0;
	buf.inUse = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// RENDERING
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiBatchRootRenderer::Render(
	VkCommandBuffer cmd,
	const GuiBatches& batches,
	const RenderTarget& target,
	VkDescriptorSet clipDescriptorSet
) {
	constexpr std::string_view functionName{ "Render" };

	const GuiRenderResources& resources{ vulkanHandler.GetGuiRenderResources() };

	// Exit early if something is not setup
	if (target.framebuffer == VK_NULL_HANDLE) { return; }
	if (shapeInstances.buffer == VK_NULL_HANDLE) {
		LogService::Log(
			LogType::ERROR, className, functionName,
			"Attempted to access shapeInstances while buffer is not initialised :: call CreateResources() first"
		);
		return;
	}
	if (clipDescriptorSet == VK_NULL_HANDLE) {
		LogService::Log(
			LogType::ERROR, className, functionName,
			"Cannot render because descriptor set is null :: SSBO may not be setup"
		);
		return;
	}
	if (batches.IsRenderListEmpty()) { return; }

	// Get number of elements per batch
	const uint32_t shapeCount{ static_cast<uint32_t>(batches.shapes.size()) };
	const uint32_t textCount{ static_cast<uint32_t>(batches.text.size()) };
	const uint32_t textureCount{ static_cast<uint32_t>(batches.textures.size()) };

	// Ensure reverse vectors are properly sized
	reversedRenderList.resize(batches.renderList.size());
	reversedShapes.resize(shapeCount);
	reversedText.resize(textCount);
	reversedTextures.resize(textureCount);

	// Reverse copy instance data
	std::reverse_copy(batches.shapes.begin(), batches.shapes.end(), reversedShapes.begin());
	std::reverse_copy(batches.text.begin(), batches.text.end(), reversedText.begin());
	std::reverse_copy(batches.textures.begin(), batches.textures.end(), reversedTextures.begin());

	// Build reversed renderList with remapped indices to match reversed instance data
	size_t outIndex{ 0 };
	for (auto it{ batches.renderList.rbegin() }; it != batches.renderList.rend();++it, ++outIndex) {
		DrawRequest req = *it;
		switch (req.type) {
		case DrawRequestType::Shape:
			if (shapeCount > 0) {
				req.instanceIndex = shapeCount - 1 - req.instanceIndex;
			}
			break;
		case DrawRequestType::Text:
			if (textCount > 0) {
				req.instanceIndex = textCount - 1 - req.instanceIndex;
			}
			break;
		case DrawRequestType::Texture:
			if (textureCount > 0) {
				req.instanceIndex = textureCount - 1 - req.instanceIndex;
			}
			break;
		default:
			break;
		}
		reversedRenderList[outIndex] = req;
	}

	// Upload shape instance data
	uint32_t maxShapeInstances{ static_cast<uint32_t>(shapeInstances.capacity / sizeof(ShapeInstance)) };
	VkDeviceSize requiredShapeBytes{ static_cast<VkDeviceSize>(reversedShapes.size()) * sizeof(ShapeInstance) };

	if (requiredShapeBytes > shapeInstances.capacity) {
		// Capacity exceeded, clamp upload and log an error
		LogService::Log(
			LogType::ERROR, className, functionName,
			"Shape instance count <" + std::to_string(reversedShapes.size()) +
			"> exceeds buffer capacity <" + std::to_string(maxShapeInstances) +
			"> :: Shapes beyond capacity will not render."
		);
		requiredShapeBytes = shapeInstances.capacity;
	}

	std::memcpy(shapeInstances.mapped, reversedShapes.data(), requiredShapeBytes);
	shapeInstances.inUse = requiredShapeBytes;

	// Begin render pass
	VkRenderPassBeginInfo rpInfo{};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.renderPass = resources.GetRenderPass();
	rpInfo.framebuffer = target.framebuffer;
	rpInfo.renderArea.offset = { 0,0 };
	rpInfo.renderArea.extent = { target.width,target.height };

	VkClearValue clearValue = { { { 0.0f,0.0f,0.0f,0.0f } } };
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
	VkRect2D scissor{ { 0,0 },{ target.width,target.height } };
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	// Push vertex-stage constants for viewport transform
	float vertPush[4]{
		static_cast<float>(target.width),
		static_cast<float>(target.height),
		0.0f, // offsetX
		0.0f  // offsetY
	};
	vkCmdPushConstants(
		cmd,
		resources.GetPipelineLayout(),
		VK_SHADER_STAGE_VERTEX_BIT,
		VERTEX_PUSH_OFFSET,
		VERTEX_PUSH_SIZE,
		vertPush
	);

	// Bind SSBO descriptor set for the full render pass
	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		resources.GetPipelineLayout(),
		DESCRIPTOR_SET_CLIP,
		1,
		&clipDescriptorSet,
		0,
		nullptr
	);

	// Bind quad buffer at binding[0] and shape instance buffer at binding[1]
	VkBuffer vertexBuffers[2]{
		resources.GetQuadBuffer(),
		shapeInstances.buffer
	};
	VkDeviceSize vertexOffsets[2]{ 0,0 };
	vkCmdBindVertexBuffers(cmd, 0, 2, vertexBuffers, vertexOffsets);

	// Iterate reversedRenderList with batching
	VkPipeline currentlyBoundPipeline{ VK_NULL_HANDLE };

	size_t i{ 0 };
	while (i < reversedRenderList.size()) {
		DrawRequestType currentType{ reversedRenderList[i].type };
		uint32_t firstInstance{ reversedRenderList[i].instanceIndex };
		uint32_t instanceCount{ 0 };

		// Count all contiguous elements in the current batch
		while (i < reversedRenderList.size() && reversedRenderList[i].type == currentType) {
			instanceCount++;
			i++;
		}

		// Switch on batch type
		switch (currentType) {
		case DrawRequestType::Shape: {
			if (firstInstance >= maxShapeInstances) {
				// Shapes are beyond capacity, skip
				continue;
			}
			// Clamp instanceCount to capacity
			uint32_t availableInstances{ maxShapeInstances - firstInstance };
			if (instanceCount > availableInstances) {
				instanceCount = availableInstances;
			}
			// Bind pipeline if not already bound
			VkPipeline shapePipeline{ resources.GetShapePipeline() };
			if (currentlyBoundPipeline != shapePipeline) {
				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shapePipeline);
				currentlyBoundPipeline = shapePipeline;
			}
			// Draw full batch
			vkCmdDraw(cmd, QUAD_VERTEX_COUNT, instanceCount, 0, firstInstance);
			break;
		}
		case DrawRequestType::Text: {
			// Stub logic
			LogService::Log(
				LogType::WIP, className, functionName,
				"Text rendering not yet implemented :: skipping " +
				std::to_string(instanceCount) + " text elements"
			);
			break;
		}
		case DrawRequestType::Texture: {
			// Stub logic
			LogService::Log(
				LogType::WIP, className, functionName,
				"Texture rendering not yet implemented :: skipping " +
				std::to_string(instanceCount) + " texture elements"
			);
			break;
		}
		default: {
			LogService::Log(
				LogType::ERROR, className, functionName,
				"Unexpected item in baggage area :: draw request is of unknown type for " +
				std::to_string(instanceCount) + " unknown elements"
			);
		}
		}
	}

	// End pass
	vkCmdEndRenderPass(cmd);
}