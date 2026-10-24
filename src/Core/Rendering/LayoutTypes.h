#pragma once

#include <map>
#include <vector>
#include <algorithm>
#include <vulkan/vulkan.h>

// 2D rectangle for drawing images to screen, must cast to int32_t and uint32_t when passed to VkRect2D
struct DrawRect {
	float x{ 0 };
	float y{ 0 };
	float width{ 0 };
	float height{ 0 };

	bool operator==(const DrawRect& other) const {
		return x == other.x && y == other.y && width == other.width && height == other.height;
	}
	bool operator!=(const DrawRect& other) const { return !(*this == other); }
};

struct Vec2 {
	float x{ 0.0f };
	float y{ 0.0f };

	Vec2(float inX = 0.0f, float inY = 0.0f) :x(inX), y(inY) {} // constructor

	// Comparison operators
	//bool operator==(const Vec2& other) const {
	//	return x == other.x &&
	//		y == other.y;
	//}
	//bool operator!=(const Vec2& other) const { return !(*this == other); }
};

struct Vec4 {
	float x{ 0.0f };
	float y{ 0.0f };
	float z{ 0.0f };
	float w{ 0.0f };

	Vec4(
		float inX = 0.0f,
		float inY = 0.0f,
		float inZ = 0.0f,
		float inW = 0.0f
	) :x(inX), y(inY), z(inZ), w(inW) {
	} // constructor

	Vec4 Clamp(float minVal, float maxVal) const {
		return Vec4{
			std::clamp(x,minVal,maxVal),
			std::clamp(y,minVal,maxVal),
			std::clamp(z, minVal, maxVal),
			std::clamp(w, minVal, maxVal)
		};
	} // Clamping utility
};

struct ScaleOffset2D {
	float offsetX{ 0.0f };
	float offsetY{ 0.0f };
	float scaleX{ 0.0f };
	float scaleY{ 0.0f };

	ScaleOffset2D(float offX = 0.0f, float offY = 0.0f, float scaX = 0.0f, float scaY = 0.0f)
		: offsetX(offX), offsetY(offY), scaleX(scaX), scaleY(scaY) {
	} // constructor
};

struct ScaleOffset1D {
	float offset{ 0.0f };
	float scale{ 0.0f };
};

// Colour structs

struct ColourRGBA_8 {
	uint8_t r{ 0 };
	uint8_t g{ 0 };
	uint8_t b{ 0 };
	uint8_t a{ 0 };
};

// Input structs
struct InputEvent {
	// store mouse input data
	// store keyboard input data
	// store pointer to uiTarget
	// store whether the UI was used (fallback to reset target)
	// store result (what exactly is needed for this?)
};

// Layout types
struct ShapeInstance {
	float centre[2]{ 0.0f, 0.0f };		// {X,Y}
	uint32_t packedBaseColour{ 0 };		// R,G,B,A
	uint32_t packedBorderColour{ 0 };	// R,G,B,A
	float sizeAndBorder[4]{ 0,0,0,0 };	// {width,height,thickness,empty}
	float radii[4]{ 0, 0, 0, 0 };		// {TL,TR,BR,BL}
	uint32_t clipStackOffset{ 0 };		// Index into SSBO for element clip start
	uint32_t clipCount{ 0 };			// Number of clips to read from SSBO
};
static_assert(sizeof(ShapeInstance) == 56, "ShapeInstance size mismatch, check struct layout");

struct TextInstance {
	// stub
};

struct TextureInstance {
	// stub
};

struct ClipRect {
	// Centre position
	float x{ 0.0f };
	float y{ 0.0f };
	// Half-size
	float halfW{ 0.0f };
	float halfH{ 0.0f };
	// Corner radii
	float rTL{ 0.0f };
	float rTR{ 0.0f };
	float rBR{ 0.0f };
	float rBL{ 0.0f };
};

enum class DrawRequestType :uint8_t {
	Undefined,
	Shape,
	Text,
	Texture
};

struct DrawRequest {
	DrawRequestType type{ DrawRequestType::Undefined }; // Which batch to draw from
	uint32_t instanceIndex{ 0 }; // Where in the buffer to get this draw's data
};

struct GuiBatches {
	// Render list
	std::vector<DrawRequest> renderList;

	// Clip rect buffer
	std::vector<ClipRect> finalClipChain; // This gets submitted to the GPU's SSBO at the rendering stage
	std::vector<ClipRect> localClipStack; // Local cache of current clipstack sample, appended to SSBO stack for each element after mutation

	// Instancing buffers store continuous data, and are appended to during tree traversal
	std::vector<ShapeInstance> shapes;
	std::vector<TextInstance> text;
	std::vector<TextureInstance> textures;

	void Clear() {
		renderList.clear();
		finalClipChain.clear();
		localClipStack.clear();
		shapes.clear();
		text.clear();
		textures.clear();
	}

	bool IsRenderListEmpty() const { return renderList.empty(); }
};

struct RenderTarget {
	VkImageView imageView{ VK_NULL_HANDLE };
	VkFramebuffer framebuffer{ VK_NULL_HANDLE };
	uint32_t width{ 0 };
	uint32_t height{ 0 };
};

struct SwapchainData {
	VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
	std::vector<VkImage> swapchainImages{};
	VkFormat swapchainImageFormat{ VK_FORMAT_UNDEFINED };
	VkExtent2D swapchainExtent{ 0,0 };
	std::vector<VkImageView> swapchainImageViews{};
};

namespace GuiConstants {
	constexpr VkFormat RGBA_8_FORMAT{ VK_FORMAT_R8G8B8A8_SRGB };
}