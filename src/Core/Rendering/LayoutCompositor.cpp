#include "LayoutCompositor.h"

#include "../Services/VulkanHandler.h"
#include "../Services/LogService.h"

#include <fstream>
#include <cstring> // Required for builing on linux

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

LayoutCompositor::LayoutCompositor(VulkanHandler& vk) : vulkanHandler(vk), guiLayout(vk) {}

LayoutCompositor::~LayoutCompositor() {
	CleanupCompositeResources();
}

void LayoutCompositor::CreateCompositeResources(VkFormat swapchainFormat) {
	constexpr std::string_view functionName{ "CreateCompositeResources" };
	LogService::Log(LogType::TRACE, className, functionName, "Initialising compositor resources");

	currentSwapchainFormat = swapchainFormat;

	CreateCommandPool();
	CreateQuadVertexBuffer();
	CreateRenderPass();
	CreateDescriptorResources();
	CreatePipeline();

	LogService::Log(LogType::SUCCESS, className, functionName, "Compositor resources initialised");
}

void LayoutCompositor::CreateFramebuffers(const std::vector<VkImageView>& swapchainImageViews, VkExtent2D swapchainExtent) {
	constexpr std::string_view functionName{ "CreateFramebuffers" };

	currentExtent = swapchainExtent;
	CleanupFramebuffers();

	// Command buffers map 1:1 with framebuffers
	AllocateCommandBuffers(swapchainImageViews.size());

	framebuffers.resize(swapchainImageViews.size());
	for (size_t i{ 0 }; i < swapchainImageViews.size();i++) {
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = compositeRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &swapchainImageViews[i];
		framebufferInfo.width = swapchainExtent.width;
		framebufferInfo.height = swapchainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(vulkanHandler.GetLogicalDevice(), &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create framebuffer");
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}

	// 1 - Destroy old pool if it exists (happens during window resize)
	if (compositeDescriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(vulkanHandler.GetLogicalDevice(), compositeDescriptorPool, nullptr);
		compositeDescriptorSets.clear();
	}

	// 2 - Create pool
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = static_cast<uint32_t>(swapchainImageViews.size());

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // Allow updating per frame
	poolInfo.maxSets = static_cast<uint32_t>(swapchainImageViews.size());
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;

	vkCreateDescriptorPool(vulkanHandler.GetLogicalDevice(), &poolInfo, nullptr, &compositeDescriptorPool);

	// 3. Allocate sets
	std::vector<VkDescriptorSetLayout> layouts(swapchainImageViews.size(), compositeDescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = compositeDescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapchainImageViews.size());
	allocInfo.pSetLayouts = layouts.data();

	compositeDescriptorSets.resize(swapchainImageViews.size());
	vkAllocateDescriptorSets(vulkanHandler.GetLogicalDevice(), &allocInfo, compositeDescriptorSets.data());
}

void LayoutCompositor::CleanupCompositeResources() {
	VkDevice device = vulkanHandler.GetLogicalDevice();

	if (compositePipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, compositePipeline, nullptr); }
	if (compositePipelineLayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, compositePipelineLayout, nullptr); }

	if (compositeDescriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(device, compositeDescriptorPool, nullptr);
		compositeDescriptorSets.clear(); // Pool destruction frees all sets
	}
	if (compositeDescriptorSetLayout != VK_NULL_HANDLE) { vkDestroyDescriptorSetLayout(device, compositeDescriptorSetLayout, nullptr); }

	if (compositeRenderPass != VK_NULL_HANDLE) { vkDestroyRenderPass(device, compositeRenderPass, nullptr); }

	if (quadVertexBufferMemory != VK_NULL_HANDLE) { vkFreeMemory(device, quadVertexBufferMemory, nullptr); }
	if (quadVertexBuffer != VK_NULL_HANDLE) { vkDestroyBuffer(device, quadVertexBuffer, nullptr); }

	if (compositeSampler != VK_NULL_HANDLE) { vkDestroySampler(device, compositeSampler, nullptr); }

	if (commandPool != VK_NULL_HANDLE) {
		vkDestroyCommandPool(device, commandPool, nullptr);
		commandBuffers.clear();
	}

	// Reset handles
	compositePipeline = VK_NULL_HANDLE;
	compositePipelineLayout = VK_NULL_HANDLE;
	compositeDescriptorPool = VK_NULL_HANDLE;
	compositeDescriptorSetLayout = VK_NULL_HANDLE;
	compositeRenderPass = VK_NULL_HANDLE;
	quadVertexBuffer = VK_NULL_HANDLE;
	quadVertexBufferMemory = VK_NULL_HANDLE;
	compositeSampler = VK_NULL_HANDLE;
	commandPool = VK_NULL_HANDLE;
}

void LayoutCompositor::CleanupFramebuffers() {
	VkDevice device = vulkanHandler.GetLogicalDevice();
	for (auto framebuffer : framebuffers) {
		if (framebuffer != VK_NULL_HANDLE) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
	}
	framebuffers.clear();
	if (!commandBuffers.empty()) {
		vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	}
	commandBuffers.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIMARY RENDERING
//////////////////////////////////////////////////////////////////////////////////////////////////////////


VkCommandBuffer LayoutCompositor::RecordCommands(uint32_t imageIndex) {
	// 1 - Ask subsystems for their latest textures and layout data
	GatherCompositingLayers();

	VkCommandBuffer cmd = commandBuffers[imageIndex];

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkResetCommandBuffer(cmd, 0);
	vkBeginCommandBuffer(cmd, &beginInfo);

	// 2 - Start render pass
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = compositeRenderPass;
	renderPassInfo.framebuffer = framebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = currentExtent;

	VkClearValue clearColour = { {{0.1f,0.1f,0.1f,0.1f}} }; // Dark grey background
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColour;

	vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// 3 - Bind static pipeline and vertex buffer
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, compositePipeline);

	VkBuffer vertexBuffers[] = { quadVertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

	// Set dynamic viewport and scissor
	VkViewport viewport{ 0.0f,0.0f,(float)currentExtent.width,(float)currentExtent.height,0.0f,1.0f };
	VkRect2D scissor{ {0,0},currentExtent };
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	// 4 - Draw each layer
	for (const auto& layer : activeLayersThisFrame) {
		if (layer.textureView == VK_NULL_HANDLE) { continue; } // Skip if subsystem didn't output texture

		// Update the descriptor set to point to this layer's texture
		UpdateDescriptorSet(imageIndex, layer.textureView);

		// Bind the descriptor set
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, compositePipelineLayout, 0, 1, &compositeDescriptorSets[imageIndex], 0, nullptr);

		// Calculate the normalised push constants (0.0 to 1.0)
		struct PushConstData {
			float offsetX, offsetY, scaleWidth, scaleHeight;
		} pushData;

		float extentWidth{ static_cast<float>(currentExtent.width) };
		float extentHeight{ static_cast<float>(currentExtent.height) };

		pushData.offsetX = layer.screenRect.x / currentExtent.width;
		pushData.offsetY = layer.screenRect.y / currentExtent.height;
		pushData.scaleWidth = layer.screenRect.width / currentExtent.width;
		pushData.scaleHeight = layer.screenRect.height / currentExtent.height;

		// Push constants to vertex shader
		vkCmdPushConstants(cmd, compositePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstData), &pushData);

		// Draw the quad (6 vertices for 2 triangles)
		vkCmdDraw(cmd, 6, 1, 0, 0);
	}

	// 5 - End
	vkCmdEndRenderPass(cmd);
	vkEndCommandBuffer(cmd);

	return cmd;
}

void LayoutCompositor::PassInputs(const InputEvent& event) {
	constexpr std::string_view functionName{ "PassInputs" };

	InputEventResult result = guiLayout.ProcessInput(event);

	if (result.inputConsumed) {
		return; // Sunk by gui
	}

	if (result.targetViewportId != UINT32_MAX) {
		auto it = viewports.find(result.targetViewportId);
		if (it != viewports.end()) {
			it->second->ProcessInput(event, result.localX, result.localY);
		}
		else {
			LogService::Log(LogType::ERROR, className, functionName,
				"Input targeted Viewport ID [" + 
				std::to_string(result.targetViewportId) +
				"], but the viewport was not found"
			);
		}
	}
	// if input not consumed and targetViewportId is default, assume neutral gui element
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// HELPER INITIALISATIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void LayoutCompositor::CreateCommandPool() {
	constexpr std::string_view functionName{ "CreateCommandPool" };

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = vulkanHandler.GetGraphicsQueueFamilyIndex();

	if (vkCreateCommandPool(vulkanHandler.GetLogicalDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create command pool");
		throw std::runtime_error("Failed to create command pool!");
	}
}

void LayoutCompositor::AllocateCommandBuffers(size_t count) {
	constexpr std::string_view functionName{ "AllocateCommandBuffers" };

	commandBuffers.resize(count);
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(count);

	if (vkAllocateCommandBuffers(vulkanHandler.GetLogicalDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to allocate command buffers");
		throw std::runtime_error("Failed to allocate command buffers!");
	}
}

void LayoutCompositor::CreateQuadVertexBuffer() {
	// 6 vertices forming a fullscreen quad in 0.0 to 1.0 space (X, Y, U, V)
	float vertices[] = {
		0.0f,  0.0f,  0.0f, 0.0f, // Top Left
		1.0f,  0.0f,  1.0f, 0.0f, // Top Right
		1.0f,  1.0f,  1.0f, 1.0f, // Bottom Right
		0.0f,  0.0f,  0.0f, 0.0f, // Top Left
		1.0f,  1.0f,  1.0f, 1.0f, // Bottom Right
		0.0f,  1.0f,  0.0f, 1.0f  // Bottom Left
	};

	VkDeviceSize bufferSize = sizeof(vertices);
	VkDevice device = vulkanHandler.GetLogicalDevice();

	// NOTE: For brevity, host-visible memory is used here. 
	// In a highly optimized engine, you would use a staging buffer and copy to GPU-local memory.
	// WIP: what are the differences between these?

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = bufferSize;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	vkCreateBuffer(device, &bufferInfo, nullptr, &quadVertexBuffer);

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, quadVertexBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	// NOTE: VulkanHandler should ideally provide a helper to find memory types. 
	allocInfo.memoryTypeIndex = vulkanHandler.FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	vkAllocateMemory(device, &allocInfo, nullptr, &quadVertexBufferMemory);
	vkBindBufferMemory(device, quadVertexBuffer, quadVertexBufferMemory, 0);

	void* data;
	vkMapMemory(device, quadVertexBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices, bufferSize);
	vkUnmapMemory(device, quadVertexBufferMemory);
}

void LayoutCompositor::CreateRenderPass() {
	constexpr std::string_view functionName{ "CreateRenderPass" };

	VkAttachmentDescription colourAttachment{};
	colourAttachment.format = currentSwapchainFormat;
	colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colourAttachmentRef{};
	colourAttachmentRef.attachment = 0;
	colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colourAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colourAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(vulkanHandler.GetLogicalDevice(), &renderPassInfo, nullptr, &compositeRenderPass) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create render pass");
		throw std::runtime_error("Failed to create render pass!");
	}
}

void LayoutCompositor::CreateDescriptorResources() {
	constexpr std::string_view functionName{ "CreateDescriptorResources" };

	VkDevice device = vulkanHandler.GetLogicalDevice();

	// Layout (1 combined image sampler per set)
	VkDescriptorSetLayoutBinding samplerBinding{};
	samplerBinding.binding = 0;
	samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBinding.descriptorCount = 1;
	samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &samplerBinding;

	vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &compositeDescriptorSetLayout);

	// Create sampler
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR; // Smooth scaling for UI
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // Prevent texture bleeding at edges
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.anisotropyEnable = VK_FALSE; // UI doesn't usually need anisotropy
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &compositeSampler) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create texture sampler");
		throw std::runtime_error("Failed to create texture sampler!");
	}

	// Note: create the pool and sets dynamically in CreateFramebuffers, since swapchain image count is not known here
}

void LayoutCompositor::CreatePipeline() {
	constexpr std::string_view functionName{ "CreatePipeline" };

	// Placeholder logic for shader loader
	auto vertShaderModule = CreateShaderModule("shaders/compositor.vert.spv");
	auto fragShaderModule = CreateShaderModule("shaders/compositor.frag.spv");

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// Vertex input: 2 floats for pos, 2 for UV
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = 4 * sizeof(float);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = 0;
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[1].offset = 2 * sizeof(float);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Dynamic viewport and scissor
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE; // Do not cull for UI quads
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Alpha blending is required for transparent UI backgrounds to work (eg overlay)
	VkPipelineColorBlendAttachmentState colourBlendAttachment{};
	colourBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colourBlendAttachment.blendEnable = VK_TRUE;
	colourBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colourBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colourBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colourBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colourBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colourBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colourBlending{};
	colourBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colourBlending.logicOpEnable = VK_FALSE;
	colourBlending.attachmentCount = 1;
	colourBlending.pAttachments = &colourBlendAttachment;

	// Push Constants
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(float) * 4; // offsetX, offsetY, scaleW, scaleH

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &compositeDescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	vkCreatePipelineLayout(vulkanHandler.GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &compositePipelineLayout);

	// Dynamic states
	std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colourBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = compositePipelineLayout;
	pipelineInfo.renderPass = compositeRenderPass;
	pipelineInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(vulkanHandler.GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &compositePipeline) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create graphics pipeline");
		throw std::runtime_error("Failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(vulkanHandler.GetLogicalDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(vulkanHandler.GetLogicalDevice(), vertShaderModule, nullptr);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// SUBSYSTEM MANAGEMENT
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void LayoutCompositor::GatherCompositingLayers() {
	activeLayersThisFrame.clear();

	// 1 - Base GUI layer (Fullscreen)
	CompositingLayer baseGui;
	baseGui.textureView = guiLayout.GetBaseTextureView();
	baseGui.screenRect = { 0.0f, 0.0f,static_cast<float>(currentExtent.width),static_cast<float>(currentExtent.height) };
	activeLayersThisFrame.push_back(baseGui);

	// 2 - Viewport layers (Subregions)
	const std::unordered_map<uint32_t, DrawRect>& rectLookupMap = guiLayout.GetViewportLayoutRequests();
	SyncViewports(rectLookupMap);

	for (const auto& [id, viewport] : viewports) {
		CompositingLayer vpLayer;
		vpLayer.textureView = viewport->GetLatestTextureView();

		if (rectLookupMap.contains(id)) {
			vpLayer.screenRect = rectLookupMap.at(id);
		}
		else {
			vpLayer.screenRect = { 0.0f,0.0f,0.0f,0.0f }; // Fallback
		}

		activeLayersThisFrame.push_back(vpLayer);
	}

	// 3 - Overlay layer (Fullscreen)
	VkImageView overlayView = guiLayout.GetOverlayTextureView();
	if (overlayView != VK_NULL_HANDLE) {
		CompositingLayer overlayLayer;
		overlayLayer.textureView = overlayView;
		overlayLayer.screenRect = { 0.0f, 0.0f, static_cast<float>(currentExtent.width),static_cast<float>(currentExtent.height) };
		activeLayersThisFrame.push_back(overlayLayer);
	}
}

void LayoutCompositor::SyncViewports(const std::unordered_map<uint32_t, DrawRect>& requestedRects) {
	// Implementation for creating/destroying unique_ptr<Viewport> based on requestedRects.panelId
	// TODO
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// VULKAN HELPERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void LayoutCompositor::UpdateDescriptorSet(uint32_t imageIndex, VkImageView textureView) {
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = textureView;
	imageInfo.sampler = compositeSampler;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = compositeDescriptorSets[imageIndex];
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(vulkanHandler.GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
}

// Placeholder - Requires implementation to read SPIR-V files
VkShaderModule LayoutCompositor::CreateShaderModule(const std::string& filename) {
	constexpr std::string_view functionName{ "CreateShaderModule" };

	// 1 - Open the file at the end (ate) and in binary mode
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		LogService::Log(LogType::CRITICAL, className, functionName,
			"Failed to open shader file: " + filename
		);
		throw std::runtime_error("Failed to open shader file: " + filename);
	}

	// 2 - Get the file size by reading the cursor position
	size_t fileSize = static_cast<size_t>(file.tellg());

	// SPIR-V require byte size to be multiple of 4
	// Early validation
	if (fileSize % 4 != 0) {
		LogService::Log(LogType::CRITICAL, className, functionName,
			"SPIR-V file size is not a multiple of 4: " + filename);
		throw std::runtime_error("Invalid SPIR-V file size!");
	}

	// 3 - Allocate a buffer and seek back to beginning of file
	std::vector<char> buffer(fileSize);
	file.seekg(0);

	// 4 - Read entire file into buffer
	file.read(buffer.data(), fileSize);
	file.close();

	// 5 - Create vulkan ShaderModule
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = buffer.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(vulkanHandler.GetLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName,
			"Failed to create shader module from file: " + filename);
		throw std::runtime_error("Failed to create shader module!");
	}

	return shaderModule;
}