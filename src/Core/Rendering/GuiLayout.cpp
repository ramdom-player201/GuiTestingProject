#include "GuiLayout.h"

#include "../Services/VulkanHandler.h"
#include "../Services/LogService.h"

#include "GuiElements/UiShape.h"

#include <cstring>

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

GuiLayout::GuiLayout(VulkanHandler& vulkanHandler)
	: vulkanHandler(vulkanHandler),
	renderer(vulkanHandler),
	clipSsbo(vulkanHandler)
{
	constexpr std::string_view functionName{ "Constructor" };

	auto testRoot{ std::make_unique<UiFrame>() };

	auto testShape1{ std::make_unique<UiShape>() };
	auto testShape2{ std::make_unique<UiShape>() };
	auto testShape3{ std::make_unique<UiShape>() };
	auto testShape4{ std::make_unique<UiShape>() };
	auto testShape5{ std::make_unique<UiShape>() };

	// root frame covers full space
	testRoot->SetSize({ 0.0f, 0.0f, 1.0f, 1.0f });

	// shape 1 top left corner
	testShape1->SetSize({ 0.0f, 0.0f, 0.5f, 0.5f });
	// default position
	testShape1->SetBaseColour({ 255,0,0,150 }); // Red, partial transparency
	testShape1->SetBorderColour({ 0,0,0,255 }); // Black, opaque
	testShape1->SetBorderThickness(5);
	testShape1->SetCornerRadii({ 50.0f,0.0f,0.0f,0.0f });

	// shape 2 bottom left corner
	testShape2->SetSize({ 0.0f, 0.0f, 0.5f, 0.5f });
	testShape2->SetPosition({ 0.0f,0.0f,0.0f,0.5f });
	testShape2->SetBaseColour({ 0,255,0,150 }); // Green, partial transparency
	testShape2->SetBorderColour({ 255,0,255,255 }); // Magenta, opaque
	testShape2->SetBorderThickness(2);
	testShape2->SetCornerRadii({ 0.0f,0.0f,0.0f,50.0f });

	// shape 3 right edge
	testShape3->SetSize({ 0.0f, 0.0f, 0.5f, 1.0f });
	testShape3->SetPosition({ 0.0f,0.0f,0.5f,0.0f });
	testShape3->SetBaseColour({ 0,0,255,150 }); // Blue, partial transparency
	testShape3->SetBorderColour({ 255,0,0,255 }); // Red, opaque
	testShape3->SetBorderThickness(15);
	testShape3->SetCornerRadii({ 0.0f,150.0f,150.0f,0.0f });

	// shape 4 centre underneath others
	testShape4->SetSize({ 0.0f,0.0f,0.5f,0.5f });
	testShape4->SetPosition({ 0.0f,0.0f,0.25f,0.25f });
	testShape4->SetBaseColour({ 255,0,0,150 }); // Red, partial transparency
	testShape4->SetBorderColour({ 0,0,0,255 }); // Black, opaque
	testShape4->SetBorderThickness(10);
	testShape4->SetCornerRadii({ 25.f,25.f,25.f,25.f });

	// Shape 5
	testShape5->SetSize({ 0.0f,0.0f,1.f,1.f });
	testShape5->SetPosition({ 0.0f,0.0f,0.5f,-0.5f });
	testShape5->SetBaseColour({ 255,255,0,255 }); // Yellow, opaque
	testShape5->SetBorderColour({ 0,0,0,0 }); // Invisible
	testShape5->SetCornerRadii({ 0.f,0.f,0.f,0.f });
	testShape3->AddChild(std::move(testShape5)); // To test clipping

	// You cannot run commands on these elements after moving them
	testRoot->AddChild(std::move(testShape4));
	testRoot->AddChild(std::move(testShape1));
	testRoot->AddChild(std::move(testShape2));
	testRoot->AddChild(std::move(testShape3));
	

	treeRoot = std::move(testRoot);
}

GuiLayout::~GuiLayout() {
	vkDeviceWaitIdle(vulkanHandler.GetLogicalDevice());
	CleanupLayerTexture();
}

void GuiLayout::InitialiseRenderResources() {
	// renderResources.CreateResources(); <- handled from VulkanHandler now
	renderer.CreateResources(8192); // handle 8192 gui elements
	clipSsbo.CreateResources(1024); // Handle 1024 clips
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
	imageInfo.format = GuiConstants::RGBA_8_FORMAT; // from LayoutTypes.h
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
	viewInfo.format = GuiConstants::RGBA_8_FORMAT;
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
	fbInfo.renderPass = vulkanHandler.GetGuiRenderResources().GetRenderPass();
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

void GuiLayout::Refresh(uint32_t width, uint32_t height) {
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

bool GuiLayout::UpdateAndRender(InputEvent& event, VkCommandBuffer cmd)
{
	constexpr std::string_view functionName{ "UpdateAndRender" };

	if (layerTexture.view == VK_NULL_HANDLE) {
		LogService::Log(LogType::ERROR, className, functionName, "Cannot process GUI without initialised texture");
		return false; // do not attempt to draw
	}

	batches.Clear(); // Clear batches from previous frame

	// Setup params for tree traversal
	UiPassParams params;
	params.event = event;
	params.activeTarget = nullptr; // TODO: must persist from last frame :: consider moving into event
	params.parentContentRect = { 0.0f,0.0f,static_cast<float>(currentWindowWidth),static_cast<float>(currentWindowHeight) };
	params.guiScaleFactor = 1.0f; // TODO: load from settings
	params.batches = &batches;
	params.layoutChanged = false;

	//LogService::Log(LogType::WIP, className, functionName, "Some UiPassParams values still need TODO work, see comments");

	// Update tree
	if (treeRoot) {
		treeRoot->ProcessElement(params);
	}

	// Upload accumulated clipChain to SSBO
	clipSsbo.Upload(batches.finalClipChain);

	// Build render target from layer texture
	RenderTarget target;
	target.imageView = layerTexture.view;
	target.framebuffer = layerTexture.framebuffer;
	target.width = currentWindowWidth;
	target.height = currentWindowHeight;

	renderer.Render(cmd, batches, target,clipSsbo.GetDescriptorSet());

	return params.layoutChanged;
}

void GuiLayout::LoadPage(PageMode mode, const std::string& layoutFilePath) {
	// WIP stub
	LogService::Log(LogType::WIP, className, "LoadPage", "LoadPage not yet implemented");
}