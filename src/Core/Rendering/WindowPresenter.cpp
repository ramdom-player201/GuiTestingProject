#include "WindowPresenter.h"

#include "../Services/VulkanHandler.h"
#include "../Services/LogService.h"

#include <fstream>
#include <cstring>

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

WindowPresenter::WindowPresenter(VulkanHandler& vk) :vulkanHandler(vk), guiLayout(vk) {
	constexpr std::string_view functionName{ "Constructor" };

	LogService::Log(LogType::TRACE, className, functionName, "Created WindowPresenter");
}

WindowPresenter::~WindowPresenter() {
	constexpr std::string_view functionName{ "Destructor" };

	LogService::Log(LogType::TRACE, className, functionName, "Destroying WindowPresenter");
	vkDeviceWaitIdle(vulkanHandler.GetLogicalDevice());
	CleanupFrameResources(); // Transient elements
	CleanupPersistentObjects(); // Persistent elements (excludes commandPool)
	if (commandPool != VK_NULL_HANDLE) { // Command pool
		vkDestroyCommandPool(vulkanHandler.GetLogicalDevice(), commandPool, nullptr);
		commandPool = VK_NULL_HANDLE;
	}
	LogService::Log(LogType::TRACE, className, functionName, "Destroyed WindowPresenter");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void WindowPresenter::Refresh(const SwapchainData& data) {
	constexpr std::string_view functionName{ "Refresh" };

	bool formatChanged = (data.swapchainImageFormat != currentSwapchainFormat); // For Pipeline/RenderPass refresh
	bool sizeChanged = (data.swapchainExtent.width != currentExtent.width || // For Framebuffer refresh and GUI resize
		data.swapchainExtent.height != currentExtent.height);

	// F|S -> P|F
	// _|_ -> _|_ <- no change
	// *|_ -> *|* <- format refresh all
	// *|* -> *|* <- all refresh all
	// _|* -> _|* <- only framebuffer

	if (!formatChanged && !sizeChanged) { return; } // No change, skip

	CleanupFrameResources(); // happens for either

	if (formatChanged) {
		LogService::Log(LogType::TRACE, className, functionName, "Swapchain format changed, refreshing pipelines");

		// Replace pipeline for format changed
		CleanupPersistentObjects();
		CreatePersistentObjects(data.swapchainImageFormat);
	}

	guiLayout.Refresh(data.swapchainExtent.width, data.swapchainExtent.height);
	bForceTextureBarrier = true; // Texture was recreated, layout is likely now UNDEFINED

	// Replace frame resources 
	LogService::Log(LogType::TRACE, className, functionName, "Swapchain size/format changed, refreshing framebuffers");
	CreateFrameResources(data);
}

VkCommandBuffer WindowPresenter::UpdateAndPresent(InputEvent& input, uint32_t imageIndex) {
	constexpr std::string_view functionName{ "UpdateAndPresent" };

	VkCommandBuffer cmd = commandBuffers[imageIndex]; // Choose command buffer for target swapchain index

	// Reset and begin command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	// Reset before recording
	vkResetCommandBuffer(cmd, 0);

	if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to begin recording command buffer");
		throw std::runtime_error("Failed to begin recording command buffer!");
		// In which cases might this error occur?
	}

	// Traverse and update tree, render if updated, return whether changed
	bool guiRefreshed = guiLayout.UpdateAndRender(input, cmd);

	VkImageView guiTextureView = guiLayout.GetTextureView();

	// Ensure texture exists and is setup
	if (guiTextureView != VK_NULL_HANDLE) {

		// rendering occured, create a barrier
		if (guiRefreshed || bForceTextureBarrier) {
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			// Layout :: ColorAttachmentOptimal OR Undefined -> ShaderReadOnlyOptimal
			if (bForceTextureBarrier) {
				barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				bForceTextureBarrier = false; // Reset flag
			}
			else {
				barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			// QueueFamilyIndex :: no change
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			// Refer to image from gui output
			barrier.image = guiLayout.GetTextureImage();
			// Subresources
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			// AccessMask :: ColorAttachmentWriteBit -> ShaderReadBit
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			// Record pipeline barrier to command buffer
			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		}

		// Apply texture to swapchain whether GUI updated or not

		// Begin render pass into swapchain
		VkRenderPassBeginInfo rpInfo{};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpInfo.renderPass = renderPass; // cached as member variable
		rpInfo.framebuffer = framebuffers[imageIndex]; // target current index in swapchain
		rpInfo.renderArea.offset = { 0, 0 };
		rpInfo.renderArea.extent = currentExtent;

		// Start by clearing the existing image
		VkClearValue clearValue = { {{0.1f, 0.1f, 0.1f, 1.0f}} }; // Dark grey background
		rpInfo.clearValueCount = 1;
		rpInfo.pClearValues = &clearValue;

		// copy gui output texture onto swapchain
		vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
		DrawFullscreenQuad(cmd, imageIndex);
		vkCmdEndRenderPass(cmd);
	}

	if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to record command buffer");
		throw std::runtime_error("Failed to record command buffer!");
		// When might this occur?
	}

	return cmd;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// SETUP HELPERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void WindowPresenter::CreatePersistentObjects(VkFormat format) {
	constexpr std::string_view functionName{ "CreatePersistentObjects" };

	currentSwapchainFormat = format;

	if (commandPool == VK_NULL_HANDLE) {
		// Created only once
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = vulkanHandler.GetGraphicsQueueFamilyIndex();

		if (vkCreateCommandPool(vulkanHandler.GetLogicalDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create command pool");
			throw std::runtime_error("Failed to create command pool");
		}
		LogService::Log(LogType::TRACE, className, functionName, "Persistent CommandPool created");

		// Also initialise GuiLayout's render resources here
		guiLayout.InitialiseRenderResources();
	}

	// Recreated on format change
	Persistent_CreateRenderPass();
	Persistent_CreateDescriptorResources();
	Persistent_CreatePipeline();
	// Cleanup of persistent objects currently occurs in the caller Refresh
	// TODO: consider renaming CreatePeristentObjects to RefreshPersistentObjects and moving cleanup call inside rather than leaving it in Refresh

	LogService::Log(LogType::SUCCESS, className, functionName, "Pipeline objects created");
}

void WindowPresenter::CreateFrameResources(const SwapchainData& data) {
	constexpr std::string_view functionName{ "RefreshFrameResources" };

	currentExtent = data.swapchainExtent;

	Frame_AllocateCommandBuffers(data.swapchainImageViews.size());

	// Create Framebuffers
	framebuffers.resize(data.swapchainImageViews.size());
	for (size_t i{ 0 }; i < data.swapchainImageViews.size(); i++) {
		VkFramebufferCreateInfo fbInfo{};
		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.renderPass = renderPass;
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = &data.swapchainImageViews[i];
		fbInfo.width = currentExtent.width;
		fbInfo.height = currentExtent.height;
		fbInfo.layers = 1;

		if (vkCreateFramebuffer(vulkanHandler.GetLogicalDevice(), &fbInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create framebuffer");
			throw std::runtime_error("Failed to create framebuffer");
		}
	}

	// Create Descriptor Pool
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = static_cast<uint32_t>(data.swapchainImageViews.size());

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.maxSets = static_cast<uint32_t>(data.swapchainImageViews.size());
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;

	if (vkCreateDescriptorPool(vulkanHandler.GetLogicalDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create descriptor pool");
		throw std::runtime_error("Failed to create descriptor pool");
	}

	// Allocate Descriptor Sets
	std::vector<VkDescriptorSetLayout> layouts(data.swapchainImageViews.size(), descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(data.swapchainImageViews.size());
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(data.swapchainImageViews.size());
	if (vkAllocateDescriptorSets(vulkanHandler.GetLogicalDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to allocate descriptor sets");
		throw std::runtime_error("Failed to allocate descriptor sets");
	}

	VkImageView guiView{ guiLayout.GetTextureView() };
	if (guiView != VK_NULL_HANDLE) {
		for (size_t i{ 0 }; i < descriptorSets.size(); i++) {
			UpdateDescriptorSet(static_cast<uint32_t>(i), guiView);
		}
	}

	LogService::Log(LogType::SUCCESS, className, functionName, "Frame resources created");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// LOW-LEVEL VULKAN HELPERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void WindowPresenter::Persistent_CreateRenderPass() {
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

	VkRenderPassCreateInfo rpInfo{};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpInfo.attachmentCount = 1;
	rpInfo.pAttachments = &colourAttachment;
	rpInfo.subpassCount = 1;
	rpInfo.pSubpasses = &subpass;
	rpInfo.dependencyCount = 1;
	rpInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(vulkanHandler.GetLogicalDevice(), &rpInfo, nullptr, &renderPass) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create render pass");
		throw std::runtime_error("Failed to create render pass");
	}
}

void WindowPresenter::Persistent_CreateDescriptorResources() {
	constexpr std::string_view functionName{ "CreateDescriptorResources" };

	VkDevice device = vulkanHandler.GetLogicalDevice();

	VkDescriptorSetLayoutBinding binding{};
	binding.binding = 0;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding.descriptorCount = 1;
	binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &binding;

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create descriptor set layout");
		throw std::runtime_error("Failed to create descriptor set layout");
	}

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create sampler");
		throw std::runtime_error("Failed to create sampler");
	}
}

void WindowPresenter::Persistent_CreatePipeline() {
	constexpr std::string_view functionName{ "CreatePipeline" };

	VkDevice device = vulkanHandler.GetLogicalDevice();


	VkShaderModule vertModule = CreateShaderModule("data/shaders/fullscreen_texture.vert.spv");
	VkShaderModule fragModule = CreateShaderModule("data/shaders/fullscreen_texture.frag.spv");

	VkPipelineShaderStageCreateInfo stages[2]{};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vertModule;
	stages[0].pName = "main";

	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = fragModule;
	stages[1].pName = "main";

	VkPipelineVertexInputStateCreateInfo vertexInput{};
	vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.vertexBindingDescriptionCount = 0;
	vertexInput.pVertexBindingDescriptions = nullptr;
	vertexInput.vertexAttributeDescriptionCount = 0;
	vertexInput.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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

	std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPushConstantRange pushConstRange{};
	pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstRange.offset = 0;
	pushConstRange.size = sizeof(float) * 4;

	VkPipelineLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = 1;
	layoutInfo.pSetLayouts = &descriptorSetLayout;
	layoutInfo.pushConstantRangeCount = 1;
	layoutInfo.pPushConstantRanges = &pushConstRange;

	if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create pipeline layout");
		throw std::runtime_error("Failed to create pipeline layout");
	}

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

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create graphics pipeline");
		throw std::runtime_error("Failed to create graphics pipeline");
	}

	vkDestroyShaderModule(device, fragModule, nullptr);
	vkDestroyShaderModule(device, vertModule, nullptr);
}

void WindowPresenter::Frame_AllocateCommandBuffers(size_t count) {
	constexpr std::string_view functionName{ "AllocateCommandBuffers" };

	commandBuffers.resize(count);
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(count);

	if (vkAllocateCommandBuffers(vulkanHandler.GetLogicalDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to allocate command buffers");
		throw std::runtime_error("Failed to allocate command buffers");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// RENDERING HELPERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void WindowPresenter::DrawFullscreenQuad(VkCommandBuffer cmd, uint32_t imageIndex) {
	constexpr std::string_view functionName{ "DrawFullscreenQuad" };

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	VkViewport viewport{ 0.0f, 0.0f, static_cast<float>(currentExtent.width), static_cast<float>(currentExtent.height), 0.0f, 1.0f };
	VkRect2D scissor{ {0, 0}, currentExtent };
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	// Update descriptors for this frame's texture
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[imageIndex], 0, nullptr);

	// Pass 1.0 scale for full screen
	struct { float offsetX, offsetY, scaleW, scaleH; } pushData{ 0.0f, 0.0f, 1.0f, 1.0f };
	vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushData), &pushData);

	vkCmdDraw(cmd, 6, 1, 0, 0);
}

void WindowPresenter::UpdateDescriptorSet(uint32_t imageIndex, VkImageView textureView) {
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = textureView;
	imageInfo.sampler = textureSampler;

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descriptorSets[imageIndex];
	write.dstBinding = 0;
	write.dstArrayElement = 0;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.descriptorCount = 1;
	write.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(vulkanHandler.GetLogicalDevice(), 1, &write, 0, nullptr);
}

VkShaderModule WindowPresenter::CreateShaderModule(const std::string& path) {
	constexpr std::string_view functionName{ "CreateShaderModule" };

	std::ifstream file(path, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to open shader: " + path);
		throw std::runtime_error("Failed to open shader: " + path);
	}

	size_t fileSize = static_cast<size_t>(file.tellg());

	if (fileSize % 4 != 0) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Invalid SPIR-V file size: " + path);
		throw std::runtime_error("Invalid SPIR-V file size");
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
	if (vkCreateShaderModule(vulkanHandler.GetLogicalDevice(), &info, nullptr, &module) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create shader module: " + path);
		throw std::runtime_error("Failed to create shader module");
	}

	return module;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLEANUP HELPERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void WindowPresenter::CleanupPersistentObjects() {
	VkDevice device = vulkanHandler.GetLogicalDevice();
	if (pipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, pipeline, nullptr); pipeline = VK_NULL_HANDLE; }
	if (pipelineLayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, pipelineLayout, nullptr); pipelineLayout = VK_NULL_HANDLE; }
	if (renderPass != VK_NULL_HANDLE) { vkDestroyRenderPass(device, renderPass, nullptr); renderPass = VK_NULL_HANDLE; }
	if (textureSampler != VK_NULL_HANDLE) { vkDestroySampler(device, textureSampler, nullptr); textureSampler = VK_NULL_HANDLE; }
	if (descriptorSetLayout != VK_NULL_HANDLE) { vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr); descriptorSetLayout = VK_NULL_HANDLE; }
}

void WindowPresenter::CleanupFrameResources() {
	VkDevice device = vulkanHandler.GetLogicalDevice();

	for (auto fb : framebuffers) {
		if (fb != VK_NULL_HANDLE) { vkDestroyFramebuffer(device, fb, nullptr); }
	}
	framebuffers.clear();

	if (!commandBuffers.empty()) {
		vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	}
	commandBuffers.clear();

	if (descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		descriptorPool = VK_NULL_HANDLE;
	}
	descriptorSets.clear();
}