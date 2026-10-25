#pragma once

#include <vulkan/vulkan.h>

#include <memory>

// Forward declarations
class VulkanHandler;
class Scene; // placeholder for now
struct InputEvent;

struct CameraTransform { // temporary
	float positionX{ 0.0f };
	float positionY{ 0.0f };
	float positionZ{ 5.0f };
	float rotationX{ 0.0f };
	float rotationY{ 0.0f };
};

class Viewport {
public:
	Viewport(VulkanHandler& vulkanHandler, std::shared_ptr<Scene> targetScene);
	~Viewport();

	// Core rendering interface
	void Render(); // NOTE: This is handled by a job manager, not by the owning window
	VkImageView GetLatestTextureView(); // this should not be written to for the duration it is used
	// NOTE: if viewport runs its own render thread from a job batch, we need a way to manage outputting the textures

	// Process input
	void ProcessInput(const InputEvent& event, float localMouseX, float localMouseY);

	// Sizing interface
	void Resize(uint32_t newWidth, uint32_t newHeight);

	// Camera interface :: WIP

	// Safety locks
	Viewport() = delete;
	Viewport(const Viewport&) = delete;
	Viewport& operator=(const Viewport&) = delete;
	Viewport(Viewport&&) = delete;
	Viewport& operator=(Viewport&&) = delete;

private:
	VulkanHandler& vulkanHandler;
	std::shared_ptr<Scene> targetScene; // Scene to render, may be shared between viewports

	CameraTransform camera;

	// Internal render target details
	uint32_t currentWidth{ 0 };
	uint32_t currentHeight{ 0 };
	VkImage renderTargetImage{ VK_NULL_HANDLE };
	VkDeviceMemory renderTargetMemory{ VK_NULL_HANDLE };
	VkImageView renderTargetView{ VK_NULL_HANDLE };

	// NOTE: future multi-threaded rendering may require 2 or 3 targets; two to alternate rendering on, and one to keep stable for external rendering
};