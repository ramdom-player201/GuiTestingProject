#include "LayoutCompositor.h"

#include "../Services/VulkanHandler.h"
#include "../Services/LogService.h"

#include <fstream>
#include <cstring>

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

LayoutCompositor::LayoutCompositor(VulkanHandler& vk) : vulkanHandler(vk), guiLayout(vk) {}

LayoutCompositor::~LayoutCompositor() {
    CleanupResources();
}

void LayoutCompositor::InitialiseGui(uint32_t width, uint32_t height) {
    constexpr std::string_view functionName{ "InitialiseGui" };

    if (width == 0 || height == 0) {
        LogService::Log(LogType::ERROR, className, functionName, "Cannot initialise with zero size");
        return;
    }

    guiLayout.Resize(width, height);

    //guiLayout.InitialiseTexture(width, height);
    LogService::Log(LogType::TRACE, className, functionName,
        "GUI initialised [" + std::to_string(width) + "x" + std::to_string(height) + "]"
    );
}

void LayoutCompositor::CreateResources(VkFormat swapchainFormat) {
    constexpr std::string_view functionName{ "CreateResources" };

    LogService::Log(LogType::TRACE, className, functionName, "Creating compositor resources");

    currentSwapchainFormat = swapchainFormat;

    guiLayout.CreateRenderResources();

    CreateCommandPool();
    CreateQuadVertexBuffer();
    CreateRenderPass();
    CreateDescriptorResources();
    CreatePipeline();

    LogService::Log(LogType::SUCCESS, className, functionName, "Compositor resources created");
}

void LayoutCompositor::CreateFramebuffers(const std::vector<VkImageView>& swapchainImageViews, VkExtent2D swapchainExtent) {
    constexpr std::string_view functionName{ "CreateFramebuffers" };

    currentExtent = swapchainExtent;
    CleanupFramebuffers();

    AllocateCommandBuffers(swapchainImageViews.size());

    framebuffers.resize(swapchainImageViews.size());
    for (size_t i{ 0 }; i < swapchainImageViews.size(); i++) {
        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = renderPass;
        fbInfo.attachmentCount = 1;
        fbInfo.pAttachments = &swapchainImageViews[i];
        fbInfo.width = swapchainExtent.width;
        fbInfo.height = swapchainExtent.height;
        fbInfo.layers = 1;

        if (vkCreateFramebuffer(vulkanHandler.GetLogicalDevice(), &fbInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create framebuffer");
            throw std::runtime_error("Failed to create framebuffer");
        }
    }

    // Destroy old pool if it exists (happens during window resize)
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(vulkanHandler.GetLogicalDevice(), descriptorPool, nullptr);
        descriptorSets.clear();
    }

    // Create Descriptor Pool
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = static_cast<uint32_t>(swapchainImageViews.size());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = static_cast<uint32_t>(swapchainImageViews.size());
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    if (vkCreateDescriptorPool(vulkanHandler.GetLogicalDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create descriptor pool");
        throw std::runtime_error("Failed to create descriptor pool");
    }

    // Allocate Descriptor Sets
    std::vector<VkDescriptorSetLayout> layouts(swapchainImageViews.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapchainImageViews.size());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(swapchainImageViews.size());
    if (vkAllocateDescriptorSets(vulkanHandler.GetLogicalDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        LogService::Log(LogType::CRITICAL, className, functionName, "Failed to allocate descriptor sets");
        throw std::runtime_error("Failed to allocate descriptor sets");
    }
}

void LayoutCompositor::CleanupResources() {
    VkDevice device = vulkanHandler.GetLogicalDevice();

    CleanupFramebuffers();

    if (pipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, pipeline, nullptr); }
    if (pipelineLayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, pipelineLayout, nullptr); }
    if (renderPass != VK_NULL_HANDLE) { vkDestroyRenderPass(device, renderPass, nullptr); }
    if (quadVertexBufferMemory != VK_NULL_HANDLE) { vkFreeMemory(device, quadVertexBufferMemory, nullptr); }
    if (quadVertexBuffer != VK_NULL_HANDLE) { vkDestroyBuffer(device, quadVertexBuffer, nullptr); }
    if (textureSampler != VK_NULL_HANDLE) { vkDestroySampler(device, textureSampler, nullptr); }

    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        descriptorSets.clear();
    }
    if (descriptorSetLayout != VK_NULL_HANDLE) { vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr); }
    if (commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, commandPool, nullptr);
        commandBuffers.clear();
    }

    // Reset handles
    pipeline = VK_NULL_HANDLE;
    pipelineLayout = VK_NULL_HANDLE;
    renderPass = VK_NULL_HANDLE;
    quadVertexBuffer = VK_NULL_HANDLE;
    quadVertexBufferMemory = VK_NULL_HANDLE;
    textureSampler = VK_NULL_HANDLE;
    descriptorPool = VK_NULL_HANDLE;
    descriptorSetLayout = VK_NULL_HANDLE;
    commandPool = VK_NULL_HANDLE;
}

void LayoutCompositor::CleanupFramebuffers() {
    VkDevice device = vulkanHandler.GetLogicalDevice();

    for (auto fb : framebuffers) {
        if (fb != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device, fb, nullptr);
        }
    }
    framebuffers.clear();

    if (!commandBuffers.empty()) {
        vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    }
    commandBuffers.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// UPDATE LOOP
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void LayoutCompositor::ProcessGui(const InputEvent& input) {
    guiLayout.ProcessGui(input);
}

VkCommandBuffer LayoutCompositor::RecordCommands(uint32_t imageIndex) {
    VkCommandBuffer cmd = commandBuffers[imageIndex];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkResetCommandBuffer(cmd, 0);
    vkBeginCommandBuffer(cmd, &beginInfo);

    VkImageView guiTexture = guiLayout.GetTextureView();
    if (guiTexture != VK_NULL_HANDLE) {
        guiLayout.Render(cmd);
    }

    VkRenderPassBeginInfo rpInfo{};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass = renderPass;
    rpInfo.framebuffer = framebuffers[imageIndex];
    rpInfo.renderArea.offset = { 0, 0 };
    rpInfo.renderArea.extent = currentExtent;

    VkClearValue clearValue = { {{0.1f, 0.1f, 0.1f, 1.0f}} };
    rpInfo.clearValueCount = 1;
    rpInfo.pClearValues = &clearValue;

    // We always begin the render pass so the screen clears to the dark grey background
    vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Only draw the quad if the GUI actually has a texture to show
    if (guiTexture != VK_NULL_HANDLE) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkBuffer vertexBuffers[] = { quadVertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

        VkViewport viewport{ 0.0f, 0.0f, static_cast<float>(currentExtent.width), static_cast<float>(currentExtent.height), 0.0f, 1.0f };
        VkRect2D scissor{ {0, 0}, currentExtent };
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        // Map the quad to the full screen extents
        struct { float offsetX, offsetY, scaleW, scaleH; } pushData{
            0.0f, 0.0f,
            static_cast<float>(currentExtent.width),
            static_cast<float>(currentExtent.height)
        };
        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushData), &pushData);

        UpdateDescriptorSet(imageIndex, guiTexture);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[imageIndex], 0, nullptr);

        vkCmdDraw(cmd, 6, 1, 0, 0);
    }

    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);

    return cmd;
}

// NOTE: might need to modify GuiLayout to expose its layoutChanged boolean to avoid redrawing when nothing has changed

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// HELPER INITIALISATIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void LayoutCompositor::CreateCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = vulkanHandler.GetGraphicsQueueFamilyIndex();

    if (vkCreateCommandPool(vulkanHandler.GetLogicalDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        LogService::Log(LogType::CRITICAL, className, "CreateCommandPool", "Failed to create command pool");
        throw std::runtime_error("Failed to create command pool");
    }
}

void LayoutCompositor::AllocateCommandBuffers(size_t count) {
    commandBuffers.resize(count);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(count);

    if (vkAllocateCommandBuffers(vulkanHandler.GetLogicalDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        LogService::Log(LogType::CRITICAL, className, "AllocateCommandBuffers", "Failed to allocate command buffers");
        throw std::runtime_error("Failed to allocate command buffers");
    }
}

void LayoutCompositor::CreateQuadVertexBuffer() {
    float vertices[] = {
        0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f
    };

    VkDeviceSize bufferSize = sizeof(vertices);
    VkDevice device = vulkanHandler.GetLogicalDevice();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &quadVertexBuffer) != VK_SUCCESS) {
        LogService::Log(LogType::CRITICAL, className, "CreateQuadVertexBuffer", "Failed to create quad vertex buffer");
        throw std::runtime_error("Failed to create quad vertex buffer");
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, quadVertexBuffer, &memReqs);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = vulkanHandler.FindMemoryType(
        memReqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    if (vkAllocateMemory(device, &allocInfo, nullptr, &quadVertexBufferMemory) != VK_SUCCESS) {
        LogService::Log(LogType::CRITICAL, className, "CreateQuadVertexBuffer", "Failed to allocate quad buffer memory");
        throw std::runtime_error("Failed to allocate quad buffer memory");
    }

    vkBindBufferMemory(device, quadVertexBuffer, quadVertexBufferMemory, 0);

    void* data;
    vkMapMemory(device, quadVertexBufferMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, vertices, bufferSize);
    vkUnmapMemory(device, quadVertexBufferMemory);
}

void LayoutCompositor::CreateRenderPass() {
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
        LogService::Log(LogType::CRITICAL, className, "CreateRenderPass", "Failed to create render pass");
        throw std::runtime_error("Failed to create render pass");
    }
}


void LayoutCompositor::CreateDescriptorResources() {
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
        LogService::Log(LogType::CRITICAL, className, "CreateDescriptorResources", "Failed to create descriptor set layout");
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
        LogService::Log(LogType::CRITICAL, className, "CreateDescriptorResources", "Failed to create sampler");
        throw std::runtime_error("Failed to create sampler");
    }
}

void LayoutCompositor::CreatePipeline() {
    VkDevice device = vulkanHandler.GetLogicalDevice();

    auto vertModule = CreateShaderModule("data/shaders/compositor.vert.spv");
    auto fragModule = CreateShaderModule("data/shaders/compositor.frag.spv");

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertModule;
    stages[0].pName = "main";

    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragModule;
    stages[1].pName = "main";

    VkVertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.stride = 4 * sizeof(float);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> attribs{};
    attribs[0].binding = 0;
    attribs[0].location = 0;
    attribs[0].format = VK_FORMAT_R32G32_SFLOAT;
    attribs[0].offset = 0;

    attribs[1].binding = 0;
    attribs[1].location = 1;
    attribs[1].format = VK_FORMAT_R32G32_SFLOAT;
    attribs[1].offset = 2 * sizeof(float);

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &bindingDesc;
    vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribs.size());
    vertexInput.pVertexAttributeDescriptions = attribs.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

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
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

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
        LogService::Log(LogType::CRITICAL, className, "CreatePipeline", "Failed to create pipeline layout");
        throw std::runtime_error("Failed to create pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = stages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &blending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        LogService::Log(LogType::CRITICAL, className, "CreatePipeline", "Failed to create graphics pipeline");
        throw std::runtime_error("Failed to create graphics pipeline");
    }

    vkDestroyShaderModule(device, fragModule, nullptr);
    vkDestroyShaderModule(device, vertModule, nullptr);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// VULKAN HELPERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void LayoutCompositor::UpdateDescriptorSet(uint32_t imageIndex, VkImageView textureView) {
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

VkShaderModule LayoutCompositor::CreateShaderModule(const std::string& path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        LogService::Log(LogType::CRITICAL, className, "CreateShaderModule", "Failed to open shader: " + path);
        throw std::runtime_error("Failed to open shader: " + path);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    if (fileSize % 4 != 0) {
        LogService::Log(LogType::CRITICAL, className, "CreateShaderModule", "Invalid SPIR-V file size: " + path);
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
        LogService::Log(LogType::CRITICAL, className, "CreateShaderModule", "Failed to create shader module: " + path);
        throw std::runtime_error("Failed to create shader module: " + path);
    }

    return module;
}