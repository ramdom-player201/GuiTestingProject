#include "GuiLayout.h"

#include "../Services/VulkanHandler.h"
#include "../Services/LogService.h"

#include <cstring>

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

GuiLayout::GuiLayout(VulkanHandler& vulkanHandler)
	: vulkanHandler(vulkanHandler),
	renderResources(vulkanHandler),
	renderer(vulkanHandler, renderResources)
{
}

GuiLayout::~GuiLayout() {
	vkDeviceWaitIdle(vulkanHandler.GetLogicalDevice());
	CleanupLayerTexture();
}

void GuiLayout::CreateRenderResources() {
	renderResources.CreateResources();
	renderer.CreateResources(8192);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// TEXTURE MANAGEMENT
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiLayout::CreateLayerTexture() {
	constexpr std::string_view functionName{ "CreateLayerTexture" };

	VkDevice device{ vulkanHandler.GetLogicalDevice() };

	// Create Image
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = currentWindowWidth;
	imageInfo.extent.height = currentWindowHeight;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = FORMAT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device, &imageInfo, nullptr, &layerTexture.image) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create layer image");
		throw std::runtime_error("Failed to create layer image");
		// When might this occur? Is this really a critical error that should result in a force quit?
	}

	// Allocate and bind memory
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, layerTexture.image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = vulkanHandler.FindMemoryType(
		memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &layerTexture.memory) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to allocate layer image memory");
		throw std::runtime_error("Failed to allocate layer image memory");
		// When might this occur? Is this really a critical error that should result in a force quit?
	}

	vkBindImageMemory(device, layerTexture.image, layerTexture.memory, 0);

	// Create ImageView
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = layerTexture.image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = FORMAT;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device, &viewInfo, nullptr, &layerTexture.view) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create layer image view");
		throw std::runtime_error("Failed to create layer image view");
		// When might this occur? Is this really a critical error that should result in a force quit?
	}

	// Create Framebuffer
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.renderPass = renderResources.GetRenderPass();
	fbInfo.attachmentCount = 1;
	fbInfo.pAttachments = &layerTexture.view;
	fbInfo.width = currentWindowWidth;
	fbInfo.height = currentWindowHeight;
	fbInfo.layers = 1;

	if (vkCreateFramebuffer(device, &fbInfo, nullptr, &layerTexture.framebuffer) != VK_SUCCESS) {
		LogService::Log(LogType::CRITICAL, className, functionName, "Failed to create layer framebuffer");
		throw std::runtime_error("Failed to create layer framebuffer");
		// When might this occur? Is this really a critical error that should result in a force quit?
	}

	LogService::Log(LogType::SUCCESS, className, functionName,
		"Layer texture created [" +
		std::to_string(currentWindowWidth) + "x" +
		std::to_string(currentWindowHeight) + "]"
	);
}

void GuiLayout::CleanupLayerTexture() {
	VkDevice device{ vulkanHandler.GetLogicalDevice() };

	if (layerTexture.framebuffer != VK_NULL_HANDLE) {
		vkDestroyFramebuffer(device, layerTexture.framebuffer, nullptr);
		layerTexture.framebuffer = VK_NULL_HANDLE;
	}
	if (layerTexture.view != VK_NULL_HANDLE) {
		vkDestroyImageView(device, layerTexture.view, nullptr);
		layerTexture.view = VK_NULL_HANDLE;
	}
	if (layerTexture.image != VK_NULL_HANDLE) {
		vkDestroyImage(device, layerTexture.image, nullptr);
		layerTexture.image = VK_NULL_HANDLE;
	}
	if (layerTexture.memory != VK_NULL_HANDLE) {
		vkFreeMemory(device, layerTexture.memory, nullptr);
		layerTexture.memory = VK_NULL_HANDLE;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void GuiLayout::Resize(uint32_t width, uint32_t height) {
	constexpr std::string_view functionName{ "Resize" };

	if (width == 0 || height == 0) {
		return;
	}
	if (width == currentWindowWidth && height == currentWindowHeight) {
		return;
	}

	LogService::Log(LogType::TRACE, className, functionName,
		"Resizing from [" +
		std::to_string(currentWindowWidth) + "x" + std::to_string(currentWindowHeight) +
		"] to [" +
		std::to_string(width) + "x" + std::to_string(height) + "]"
	);

	vkDeviceWaitIdle(vulkanHandler.GetLogicalDevice());
	// ^ Pauses execution until GPU is idle before attempting to replace textures

	CleanupLayerTexture();

	currentWindowWidth = width;
	currentWindowHeight = height;

	CreateLayerTexture();
}

InputEventResult GuiLayout::ProcessGui(const InputEvent& event) {
	constexpr std::string_view functionName{ "ProcessGui" };

	if (layerTexture.view == VK_NULL_HANDLE) {
		LogService::Log(LogType::ERROR, className, functionName, "Cannot process GUI without initialised texture");
		return InputEventResult();
	}

	batches.Clear(); // Clear batches from previous frame

	// Setup params for tree traversal
	UiPassParams params;
	params.event = event;
	params.activeTarget = nullptr; // TODO: must persist from last frame
	params.parentContentRect = { 0.0f,0.0f,static_cast<float>(currentWindowWidth),static_cast<float>(currentWindowHeight) };
	params.guiScaleFactor = 1.0f; // TODO: load from settings
	params.batches = &batches;
	params.layoutChanged = false;

	//LogService::Log(LogType::WIP, className, functionName, "Some UiPassParams values still need TODO work, see comments");

	if (treeRoot) {
		treeRoot->ProcessElement(params);
	}

	//LogService::Log(LogType::WIP, className, functionName, "InputEventResult might need to be deprecated");
	// ^ We no longer store Viewports externally, so its original function is null unless we need it for something else

	return params.result;
}

void GuiLayout::Render(VkCommandBuffer cmd) {
	constexpr std::string_view functionName{ "Render" };

	if (layerTexture.view == VK_NULL_HANDLE) {
		LogService::Log(LogType::ERROR, className, functionName, "Cannot render without initialised texture");
		return;
	}

	// Build render target from layer texture
	RenderTarget target;
	target.imageView = layerTexture.view;
	target.framebuffer = layerTexture.framebuffer;
	target.width = currentWindowWidth;
	target.height = currentWindowHeight;

	renderer.Render(cmd, batches, target);
}

void GuiLayout::LoadPage(PageMode mode, const std::string& layoutFilePath) {
	// WIP stub
	LogService::Log(LogType::WIP, className, "LoadPage", "LoadPage not yet implemented");
}