#include "GuiRenderResources.h"

#include "LayoutTypes.h"
#include "../Services/LogService.h"

#include <fstream>
#include <vector>
#include <array>
#include <cstddef> // offsetof
#include <cstring> // memcpy
#include <stdexcept> // std::runtime_error

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANTS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace { // Guard against multiple-definition errors
	constexpr std::string_view SHAPE_VERT_PATH{ "data/shaders/gui_shape.vert.spv" };
	constexpr std::string_view SHAPE_FRAG_PATH{ "data/shaders/gui_shape.frag.spv" };

	// Push constant layout, only used by vertex stage
	constexpr uint32_t VERTEX_PUSH_OFFSET{ 0 };
	constexpr uint32_t VERTEX_PUSH_SIZE{ 16 }; // width, height, offsetX, offsetY

	// SSBO descriptor binding
	constexpr uint32_t SSBO_BINDING{ 0 };
	constexpr uint32_t SSBO_SET{ 0 };

	// Descriptor pool sizing
	// 1 descriptor set per GuiLayout (via GuiClipSsbo) for its clipping SSBO
	constexpr uint32_t MAX_DESCRIPTOR_SETS{ 32 };
	constexpr uint32_t MAX_SSBO_BINDINGS{ 32 };
	// Limit of 32 concurrent GuiLayouts (this is most likely to be relevant in a 3D scene, but that comes later)
	// If exhausted, would this be a CRITICAL (crash-worthy) or just ERROR (safely skip element and render blank)

	// Static unit quad vertices
	// 4 vertices * 2 floats * 4 bytes = 32 bytes
	constexpr float QUAD_VERTICES[4][2] = {
		{ -1.0f, -1.0f },	// bottom-left
		{ -1.0f,  1.0f },	// top-left
		{ 1.0f, -1.0f },	// bottom-right
		{ 1.0f,  1.0f },	// top-right
	};
	constexpr VkDeviceSize QUAD_BUFFER_SIZE{ sizeof(QUAD_VERTICES) }; // 32 bytes
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

GuiRenderResources::GuiRenderResources() {
	LogService::Log(LogType::TRACE, className, "Constructor", "Created GuiRenderResources");
}

GuiRenderResources::~GuiRenderResources() {
	LogService::Log(LogType::TRACE, className, "Destructor", "Destroyed GuiRenderResources");
}

void GuiRenderResources::Initialise(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice logicalDevice) {
	constexpr std::string_view functionName{ "Initialise" };

	if (initialised) {
		LogService::Log(LogType::ABNORM, className, functionName, "Initialise called twice - ignoring");
		return;
	}

	vkInstance = instance;
	vkPhysicalDevice = physicalDevice;
	vkLogicalDevice = logicalDevice;

	Create_RenderPass();
	Create_Sampler();
	Create_SsboDescriptorSetLayout();
	Create_SsboDescriptorPool();
	Create_PipelineLayout();
	Create_QuadBuffer();

	pipelineShapes = Create_Pipeline(SHAPE_VERT_PATH, SHAPE_FRAG_PATH);
	pipelineText = VK_NULL_HANDLE;	   // stub
	pipelineTextures = VK_NULL_HANDLE; // stub

	initialised = true;

	LogService::Log(LogType::SUCCESS, className, functionName, "GuiRenderResources initialised");
}

void GuiRenderResources::Cleanup() {
	constexpr std::string_view functionName{ "Cleanup" };

	if (vkLogicalDevice == VK_NULL_HANDLE) {
		LogService::Log(LogType::ABNORM, className, functionName, "Failed to cleanup GuiRenderResources because missing logical device");
		return;
	}

	vkDeviceWaitIdle(vkLogicalDevice);

	if (pipelineShapes != VK_NULL_HANDLE) { vkDestroyPipeline(vkLogicalDevice, pipelineShapes, nullptr);								pipelineShapes = VK_NULL_HANDLE; }
	if (pipelineText != VK_NULL_HANDLE) { vkDestroyPipeline(vkLogicalDevice, pipelineText, nullptr);									pipelineText = VK_NULL_HANDLE; }
	if (pipelineTextures != VK_NULL_HANDLE) { vkDestroyPipeline(vkLogicalDevice, pipelineTextures, nullptr);							pipelineTextures = VK_NULL_HANDLE; }
	if (pipelineLayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(vkLogicalDevice, pipelineLayout, nullptr);							pipelineLayout = VK_NULL_HANDLE; }
	if (ssboDescriptorPool != VK_NULL_HANDLE) { vkDestroyDescriptorPool(vkLogicalDevice, ssboDescriptorPool, nullptr);					ssboDescriptorPool = VK_NULL_HANDLE; }
	if (ssboDescriptorSetLayout != VK_NULL_HANDLE) { vkDestroyDescriptorSetLayout(vkLogicalDevice, ssboDescriptorSetLayout, nullptr);	ssboDescriptorSetLayout = VK_NULL_HANDLE; }
	if (quadBuffer != VK_NULL_HANDLE) { vkDestroyBuffer(vkLogicalDevice, quadBuffer, nullptr);											quadBuffer = VK_NULL_HANDLE; }
	if (quadBufferMemory != VK_NULL_HANDLE) { vkFreeMemory(vkLogicalDevice, quadBufferMemory, nullptr);									quadBufferMemory = VK_NULL_HANDLE; }
	if (textureSampler != VK_NULL_HANDLE) { vkDestroySampler(vkLogicalDevice, textureSampler, nullptr);									textureSampler = VK_NULL_HANDLE; }
	if (pipelineRenderPass != VK_NULL_HANDLE) { vkDestroyRenderPass(vkLogicalDevice, pipelineRenderPass, nullptr);						pipelineRenderPass = VK_NULL_HANDLE; }

	initialised = false;

	LogService::Log(LogType::TRACE, className, functionName, "Cleaned GuiRenderResources");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY HELPER
//////////////////////////////////////////////////////////////////////////////////////////////////////////


uint32_t GuiRenderResources::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
	constexpr std::string_view functionName{ "FindMemoryType" };

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memProperties);

	for (uint32_t i{ 0 }; i < memProperties.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	LogService::Log(LogType::CRITICAL, className, functionName, "Failed to find suitable memory type");
	throw std::runtime_error("Failed to find suitable memory type");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// RENDER PASS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiRenderResources::Create_RenderPass() {
	constexpr std::string_view functionName{ "Create_RenderPass" };

	VkAttachmentDescription colourAttachment{};
	colourAttachment.format = GuiConstants::RGBA_8_FORMAT;
	colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference colourRef{};
	colourRef.attachment = 0;
	colourRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colourRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = 1;
	info.pAttachments = &colourAttachment;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;
	info.dependencyCount = 1;
	info.pDependencies = &dependency;

	if (vkCreateRenderPass(vkLogicalDevice, &info, nullptr, &pipelineRenderPass) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create render pass");
		throw std::runtime_error("Failed to create render pass");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// SAMPLER
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiRenderResources::Create_Sampler() {
	constexpr std::string_view functionName{ "Create_Sampler" };

	VkSamplerCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.magFilter = VK_FILTER_LINEAR;
	info.minFilter = VK_FILTER_LINEAR;
	info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	info.anisotropyEnable = VK_FALSE;
	info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	info.unnormalizedCoordinates = VK_FALSE;
	info.compareEnable = VK_FALSE;
	info.compareOp = VK_COMPARE_OP_ALWAYS;
	info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	info.mipLodBias = 0.0f;
	info.minLod = 0.0f;
	info.maxLod = 0.0f;

	if (vkCreateSampler(vkLogicalDevice, &info, nullptr, &textureSampler) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create sampler");
		throw std::runtime_error("Failed to create sampler");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// SSBO DESCRIPTOR SET LAYOUT
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiRenderResources::Create_SsboDescriptorSetLayout() {
	constexpr std::string_view functionName{ "Create_SsboDescriptorSetLayout" };

	VkDescriptorSetLayoutBinding ssboBinding{};
	ssboBinding.binding = SSBO_BINDING;
	ssboBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	ssboBinding.descriptorCount = 1;
	ssboBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	// pImmutableSamplers = nullptr (not used for storage buffers)

	VkDescriptorSetLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = 1;
	info.pBindings = &ssboBinding;

	if (vkCreateDescriptorSetLayout(vkLogicalDevice, &info, nullptr, &ssboDescriptorSetLayout) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create SSBO descriptor set layout");
		throw std::runtime_error("Failed to create SSBO descriptor set layout");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// SSBO DESCRIPTOR POOL
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiRenderResources::Create_SsboDescriptorPool() {
	constexpr std::string_view functionName{ "Create_SsboDescriptorPool" };

	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSize.descriptorCount = MAX_SSBO_BINDINGS;

	VkDescriptorPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;  // Allow individual set freeing
	info.maxSets = MAX_DESCRIPTOR_SETS;
	info.poolSizeCount = 1;
	info.pPoolSizes = &poolSize;

	if (vkCreateDescriptorPool(vkLogicalDevice, &info, nullptr, &ssboDescriptorPool) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create SSBO descriptor pool");
		throw std::runtime_error("Failed to create SSBO descriptor pool");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// SSBO DESCRIPTOR SET ALLOCATION
//////////////////////////////////////////////////////////////////////////////////////////////////////////

VkDescriptorSet GuiRenderResources::AllocateSsboDescriptorSet(VkBuffer ssboBuffer, VkDeviceSize ssboSize) const {
	constexpr std::string_view functionName{ "AllocateSsboDescriptorSet" };

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = ssboDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &ssboDescriptorSetLayout;

	VkDescriptorSet set{ VK_NULL_HANDLE };
	VkResult result{ vkAllocateDescriptorSets(vkLogicalDevice, &allocInfo, &set) };

	if (result == VK_ERROR_OUT_OF_POOL_MEMORY) {
		LogService::Log(LogType::ERROR, className, functionName,
			"SSBO descriptor pool exhausted (32 concurrent GuiLayouts max) :: Failed to allocate");
		return VK_NULL_HANDLE;
	}
	else if (result != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to allocate descriptor set");
		throw std::runtime_error("Failed to allocate descriptor set");
	}

	// Configure the descriptor set to point at the provided SSBO buffer
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = ssboBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = ssboSize;

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = set;
	write.dstBinding = SSBO_BINDING;
	write.dstArrayElement = 0;
	write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	write.descriptorCount = 1;
	write.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(vkLogicalDevice, 1, &write, 0, nullptr);

	return set;
}

void GuiRenderResources::FreeSsboDescriptorSet(VkDescriptorSet set) const {
	if (set == VK_NULL_HANDLE) { return; }
	vkFreeDescriptorSets(vkLogicalDevice, ssboDescriptorPool, 1, &set);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// PIPELINE LAYOUT
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiRenderResources::Create_PipelineLayout() {
	constexpr std::string_view functionName{ "Create_PipelineLayout" };

	// Set 0: SSBO descriptor set (shared across all GUI pipelines)
	VkDescriptorSetLayout setLayouts[] = { ssboDescriptorSetLayout };

	// Push constant: vertex-stage viewport transform (16 bytes)
	VkPushConstantRange vertPushRange{};
	vertPushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	vertPushRange.offset = VERTEX_PUSH_OFFSET;
	vertPushRange.size = VERTEX_PUSH_SIZE;

	VkPipelineLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.setLayoutCount = 1;
	info.pSetLayouts = setLayouts;
	info.pushConstantRangeCount = 1;
	info.pPushConstantRanges = &vertPushRange;

	if (vkCreatePipelineLayout(vkLogicalDevice, &info, nullptr, &pipelineLayout) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create pipeline layout");
		throw std::runtime_error("Failed to create pipeline layout");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// QUAD BUFFER (Static unit quad for instanced shape rendering)
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiRenderResources::Create_QuadBuffer() {
	constexpr std::string_view functionName{ "Create_QuadBuffer" };

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = QUAD_BUFFER_SIZE;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(vkLogicalDevice, &bufferInfo, nullptr, &quadBuffer) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create quad buffer");
		throw std::runtime_error("Failed to create quad buffer");
	}

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(vkLogicalDevice, quadBuffer, &memReqs);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = FindMemoryType(
		memReqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	if (vkAllocateMemory(vkLogicalDevice, &allocInfo, nullptr, &quadBufferMemory) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to allocate quad buffer memory");
		throw std::runtime_error("Failed to allocate quad buffer memory");
	}

	vkBindBufferMemory(vkLogicalDevice, quadBuffer, quadBufferMemory, 0);

	// Write static quad data once during init
	void* mapped{ nullptr };
	vkMapMemory(vkLogicalDevice, quadBufferMemory, 0, QUAD_BUFFER_SIZE, 0, &mapped);
	std::memcpy(mapped, QUAD_VERTICES, QUAD_BUFFER_SIZE);
	vkUnmapMemory(vkLogicalDevice, quadBufferMemory);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// PIPELINE CREATION
//////////////////////////////////////////////////////////////////////////////////////////////////////////

VkPipeline GuiRenderResources::Create_Pipeline(std::string_view vertPath, std::string_view fragPath) {
	constexpr std::string_view functionName{ "Create_Pipeline" };

	auto vertModule{ Create_ShaderModule(vertPath) };
	auto fragModule{ Create_ShaderModule(fragPath) };

	VkPipelineShaderStageCreateInfo stages[2]{};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vertModule;
	stages[0].pName = "main";

	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = fragModule;
	stages[1].pName = "main";

	// Two vertex bindings:
	//   binding 0: per-vertex unit quad (8 bytes stride)
	//   binding 1: per-instance ShapeInstance (56 bytes stride)
	VkVertexInputBindingDescription bindings[2]{};
	bindings[0].binding = 0;
	bindings[0].stride = 2 * sizeof(float);
	bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	bindings[1].binding = 1;
	bindings[1].stride = sizeof(ShapeInstance);
	bindings[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	// Attribute descriptions match gui_shape.vert locations 0-7
	std::array<VkVertexInputAttributeDescription, 8> attributes{};

	// location 0: inUnitPos (vec2, binding 0 - the quad)
	attributes[0].location = 0;
	attributes[0].binding = 0;
	attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributes[0].offset = 0;

	// location 1: inCentre (vec2, binding 1 - per-instance)
	attributes[1].location = 1;
	attributes[1].binding = 1;
	attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributes[1].offset = offsetof(ShapeInstance, centre);

	// location 2: inBaseColour (vec4, UNORM unpacked from uint32)
	attributes[2].location = 2;
	attributes[2].binding = 1;
	attributes[2].format = VK_FORMAT_R8G8B8A8_UNORM;
	attributes[2].offset = offsetof(ShapeInstance, packedBaseColour);

	// location 3: inBorderColour (vec4, UNORM unpacked from uint32)
	attributes[3].location = 3;
	attributes[3].binding = 1;
	attributes[3].format = VK_FORMAT_R8G8B8A8_UNORM;
	attributes[3].offset = offsetof(ShapeInstance, packedBorderColour);

	// location 4: inSizeBorder (vec4)
	attributes[4].location = 4;
	attributes[4].binding = 1;
	attributes[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributes[4].offset = offsetof(ShapeInstance, sizeAndBorder);

	// location 5: inRadii (vec4)
	attributes[5].location = 5;
	attributes[5].binding = 1;
	attributes[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributes[5].offset = offsetof(ShapeInstance, radii);

	// location 6: inClipStackOffset (uint)
	attributes[6].location = 6;
	attributes[6].binding = 1;
	attributes[6].format = VK_FORMAT_R32_UINT;
	attributes[6].offset = offsetof(ShapeInstance, clipStackOffset);

	// location 7: inClipCount (uint)
	attributes[7].location = 7;
	attributes[7].binding = 1;
	attributes[7].format = VK_FORMAT_R32_UINT;
	attributes[7].offset = offsetof(ShapeInstance, clipCount);

	VkPipelineVertexInputStateCreateInfo vertexInput{};
	vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.vertexBindingDescriptionCount = 2;
	vertexInput.pVertexBindingDescriptions = bindings;
	vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
	vertexInput.pVertexAttributeDescriptions = attributes.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasteriser{};
	rasteriser.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasteriser.depthClampEnable = VK_FALSE;
	rasteriser.rasterizerDiscardEnable = VK_FALSE;
	rasteriser.polygonMode = VK_POLYGON_MODE_FILL;
	rasteriser.lineWidth = 1.0f;
	rasteriser.cullMode = VK_CULL_MODE_NONE;
	rasteriser.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasteriser.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState blendAttachment{};
	blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttachment.blendEnable = VK_TRUE;
	blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo blending{};
	blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blending.logicOpEnable = VK_FALSE;
	blending.attachmentCount = 1;
	blending.pAttachments = &blendAttachment;

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = stages;
	pipelineInfo.pVertexInputState = &vertexInput;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasteriser;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &blending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = pipelineRenderPass;
	pipelineInfo.subpass = 0;

	VkPipeline pipeline{ VK_NULL_HANDLE };
	if (vkCreateGraphicsPipelines(vkLogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create pipeline");
		throw std::runtime_error("Failed to create pipeline");
	}

	vkDestroyShaderModule(vkLogicalDevice, fragModule, nullptr);
	vkDestroyShaderModule(vkLogicalDevice, vertModule, nullptr);

	return pipeline;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// SHADER MODULE LOADING
//////////////////////////////////////////////////////////////////////////////////////////////////////////

VkShaderModule GuiRenderResources::Create_ShaderModule(std::string_view path) {
	constexpr std::string_view functionName{ "Create_ShaderModule" };

	const std::string pathStr(path);

	std::ifstream file(pathStr, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to open shader: " + pathStr);
		throw std::runtime_error("Failed to open shader: " + pathStr);
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	if (fileSize % 4 != 0) {
		LogService::Log(LogType::CRITICAL, className, functionName, "SPIR-V size not multiple of 4: " + pathStr);
		throw std::runtime_error("Invalid SPIR-V file");
	}

	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	VkShaderModuleCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = buffer.size();
	info.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

	VkShaderModule module{ VK_NULL_HANDLE };
	if (vkCreateShaderModule(vkLogicalDevice, &info, nullptr, &module) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create shader module: " + pathStr);
		throw std::runtime_error("Failed to create shader module");
	}

	return module;
}