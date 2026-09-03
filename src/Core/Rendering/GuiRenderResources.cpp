#include "GuiRenderResources.h"

#include "LayoutTypes.h"

#include "../Services/VulkanHandler.h"
#include "../Services/LogService.h"

#include <fstream>
#include <vector>

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANTS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace { // restrict contents to just this cpp file
	constexpr std::string_view SHAPE_VERT_PATH{ "data/shaders/gui_shape.vert.spv" };
	constexpr std::string_view SHAPE_FRAG_PATH{ "data/shaders/gui_shape.frag.spv" };

	// Push constant layout partitioning
	constexpr uint32_t VERTEX_PUSH_OFFSET{ 0 };
	constexpr uint32_t VERTEX_PUSH_SIZE{ 16 }; // width, height, offsetX, offsetY

	constexpr uint32_t FRAGMENT_PUSH_OFFSET{ 16 };
	constexpr uint32_t MAX_CLIP_RECTS{ 3 }; // Why 3 maximum?
	constexpr uint32_t FRAGMENT_PUSH_SIZE{
		MAX_CLIP_RECTS * sizeof(ClipRect) + // 3 * 32 = 96
		sizeof(uint32_t) +					// clipCount = 4 <- why 4?
		12									// alignment padding
	}; // 112 byte total? (to be verified) -> Add LogService::Log() assertion
	// 112 + 16 = 128
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

GuiRenderResources::GuiRenderResources(VulkanHandler& vk) : vulkanHandler(vk) {
	constexpr std::string_view functionName{ "Constructor" };
}

void GuiRenderResources::CreateResources() {
	constexpr std::string_view functionName{ "CreateResources" };

	Cleanup(); // Ensure it is fresh if recreating due to resize

	VkDevice device{ vulkanHandler.GetLogicalDevice() };

	CreateRenderPass();
	CreateSampler();
	CreatePipelineLayout();
	shapePipeline = CreatePipeline(SHAPE_VERT_PATH, SHAPE_FRAG_PATH, false);

	textPipeline = VK_NULL_HANDLE; // stub
	texturePipeline = VK_NULL_HANDLE; // stub

	LogService::Log(LogType::SUCCESS, className, functionName, "GUI render resources created");
}

void GuiRenderResources::Cleanup()
{
	constexpr std::string_view functionName{ "Cleanup" };

	VkDevice device{ vulkanHandler.GetLogicalDevice() };
	if (device == VK_NULL_HANDLE) return; // Safety check

	vkDeviceWaitIdle(device); // Ensure GPU isn't using resources

	if (shapePipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, shapePipeline, nullptr); shapePipeline = VK_NULL_HANDLE; }
	if (textPipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, textPipeline, nullptr); textPipeline = VK_NULL_HANDLE; }
	if (texturePipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, texturePipeline, nullptr); texturePipeline = VK_NULL_HANDLE; }
	if (pipelineLayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, pipelineLayout, nullptr); pipelineLayout = VK_NULL_HANDLE; }
	if (textureSampler != VK_NULL_HANDLE) { vkDestroySampler(device, textureSampler, nullptr); textureSampler = VK_NULL_HANDLE; }
	if (renderPass != VK_NULL_HANDLE) { vkDestroyRenderPass(device, renderPass, nullptr); renderPass = VK_NULL_HANDLE; }

	LogService::Log(LogType::TRACE, className, functionName, "GUI render resources destroyed");
}

GuiRenderResources::~GuiRenderResources() {
	Cleanup();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// RENDER PASS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiRenderResources::CreateRenderPass() {
	constexpr std::string_view functionName{ "CreateRenderPass" };

	VkDevice device{ vulkanHandler.GetLogicalDevice() };

	VkAttachmentDescription colourAttachment{};
	colourAttachment.format = VK_FORMAT_R8G8B8A8_SRGB; // Is the format the same everywhere? Should we move this to a shared header.
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

	if (vkCreateRenderPass(device, &info, nullptr, &renderPass) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create render pass");
		throw std::runtime_error("Failed to create render pass");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// SAMPLER
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiRenderResources::CreateSampler() {
	constexpr std::string_view functionName{ "CreateSampler" };

	VkDevice device{ vulkanHandler.GetLogicalDevice() };

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

	if (vkCreateSampler(device, &info, nullptr, &textureSampler) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create sampler");
		throw std::runtime_error("Failed to create sampler");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// PIPELINE LAYOUT
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiRenderResources::CreatePipelineLayout() {
	constexpr std::string_view functionName{ "CreatePipelineLayout" };

	VkDevice device{ vulkanHandler.GetLogicalDevice() };

	VkPushConstantRange ranges[2]{};

	// Vertex stage -> viewport transform
	ranges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	ranges[0].offset = VERTEX_PUSH_OFFSET;
	ranges[0].size = VERTEX_PUSH_SIZE;

	// Fragment stage -> clip stack
	ranges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	ranges[1].offset = FRAGMENT_PUSH_OFFSET;
	ranges[1].size = FRAGMENT_PUSH_SIZE;

	VkPipelineLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.pushConstantRangeCount = 2;
	info.pPushConstantRanges = ranges;

	// Note: Shape pipeline has no descriptor set layouts as no texture sampling

	if (vkCreatePipelineLayout(device, &info, nullptr, &pipelineLayout) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create pipeline layout");
		throw std::runtime_error("Failed to create pipeline layout");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// PIPELINE CREATION
//////////////////////////////////////////////////////////////////////////////////////////////////////////

VkPipeline GuiRenderResources::CreatePipeline(std::string_view vertPath, std::string_view fragPath, bool hasSampler) {
	constexpr std::string_view functionName{ "CreatePipeline" };

	VkDevice device{ vulkanHandler.GetLogicalDevice() };

	auto vertModule{ CreateShaderModule(vertPath) };
	auto fragModule{ CreateShaderModule(fragPath) };

	// Note, do we need to pass device into CreatePipeline?

	VkPipelineShaderStageCreateInfo stages[2]{};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vertModule;
	stages[0].pName = "main";

	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = fragModule;
	stages[1].pName = "main";

	// Vertex input for ShapeVertex layout
	// pos[2] + centre[2] + packedBaseColour + packedBorderColour + sizeAndBorder[4] + radii[4]
	std::array<VkVertexInputAttributeDescription, 6> attributes{};

	attributes[0].location = 0;
	attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributes[0].offset = 0; // pos

	attributes[1].location = 1;
	attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributes[1].offset = 8; // centre

	attributes[2].location = 2;
	attributes[2].format = VK_FORMAT_R8G8B8A8_UNORM;
	attributes[2].offset = 16; // packedBaseColour

	attributes[3].location = 3;
	attributes[3].format = VK_FORMAT_R8G8B8A8_UNORM;
	attributes[3].offset = 20; // packedBorderColour

	attributes[4].location = 4;
	attributes[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributes[4].offset = 24; // sizeAndBorder

	attributes[5].location = 5;
	attributes[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributes[5].offset = 40; // radii

	VkVertexInputBindingDescription binding{};
	binding.binding = 0;
	binding.stride = sizeof(ShapeVertex); // 56 bytes (do we need a LogService log assertion?)
	binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkPipelineVertexInputStateCreateInfo vertexInput{};
	vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.vertexBindingDescriptionCount = 1;
	vertexInput.pVertexBindingDescriptions = &binding;
	vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
	vertexInput.pVertexAttributeDescriptions = attributes.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Dynamic viewport and scissor
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

	// Alpha blending
	VkPipelineColorBlendAttachmentState blendAttachment{};
	blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttachment.blendEnable = VK_TRUE;
	blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
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
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	VkPipeline pipeline{ VK_NULL_HANDLE };
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create pipeline");
		throw std::runtime_error("Failed to create pipeline");
	}

	vkDestroyShaderModule(device, fragModule, nullptr);
	vkDestroyShaderModule(device, vertModule, nullptr);

	return pipeline;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// SHADER MODULE LOADING
//////////////////////////////////////////////////////////////////////////////////////////////////////////

VkShaderModule GuiRenderResources::CreateShaderModule(std::string_view path) {
	constexpr std::string_view functionName{ "CreateShaderModule" };

	const std::string pathStr(path);
	VkDevice device{ vulkanHandler.GetLogicalDevice() };

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
	if (vkCreateShaderModule(device, &info, nullptr, &module) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create shader module: " + pathStr);
		throw std::runtime_error("Failed to create shader module");
	}

	return module;
}