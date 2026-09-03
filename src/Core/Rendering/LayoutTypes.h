#pragma once

#include <map>
#include <vector>
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
};

//// A single axis value unioning a ratio and absolute value
//struct UDim {
//	float scale{ 0.0f }; // 0.0 to 1.0 (ratio of parent size
//	float offset{ 0.0f }; // Absolute pixels, but multiplied by scale
//
//	UDim(float inScale = 0.0f, float inOffset = 0.0f) :scale(inScale), offset(inOffset) {} // Constructor
//
//	// Comparison operators
//	//bool operator==(const UDim& other) const {
//	//	return scale == other.scale && offset == other.offset;
//	//}
//	//bool operator!=(const UDim& other) const { return !(*this == other); }
//};
//
//// 2D coordinate or size
//struct UDim2 {
//	UDim x;
//	UDim y;
//
//	UDim2(float scaleX = 0.0f, float offsetX = 0.0f, float scaleY = 0.0f, float offsetY = 0.0f) :
//		x(scaleX, offsetX), y(scaleY, offsetY) {
//	} // Constructor from raw values
//	UDim2(UDim inX, UDim inY) :x(inX), y(inY) {} // Constructor from UDim pair
//
//	// Comparison operators
//	//bool operator==(const UDim2& other) const {
//	//	return x == other.x && y == other.y;
//	//}
//	//bool operator!=(const UDim2& other) const { return !(*this == other); }
//};

struct ScaleOffset2D {
	float offsetX{ 0.0f };
	float offsetY{ 0.0f };
	float scaleX{ 0.0f };
	float scaleY{ 0.0f };
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

};

struct InputEventResult {
	bool inputConsumed{ false }; // Is the input consumed by the gui, or does it need to be passed to a viewport?
	uint32_t targetViewportId{ UINT32_MAX }; // Max acts as null value (no viewport)
	// Mouse coordinates relative to the viewport instead of screen (only valid if viewport)
	float localX{ 0.0f };
	float localY{ 0.0f };
};

// Layout types
struct ShapeVertex {
	float pos[2]; // {X,Y}
	float centre[2]; // {X,Y}
	uint32_t packedBaseColour; // A,B,G,R
	uint32_t packedBorderColour; // A,B,G,R
	float sizeAndBorder[4]; // {width,height,thickness,empty}
	float radii[4]; // {TL,TR,BR,BL}
};
static_assert(sizeof(ShapeVertex) == 56, "ShapeVertex size mismatch, check struct layout");

struct TextVertex {
	// stub
};

struct TextureVertex {
	// stub
};

struct ClipRect {
	float x, y;					// Centre position
	float halfW, halfH;			// Half-size
	float rTL, rTR, rBR, rBL;	// Corner radii
};

struct ClipStack {
	std::vector<ClipRect> rects;

	uint32_t Count() const { return static_cast<uint32_t>(rects.size()); }
	size_t DataSize() const { return rects.size() * sizeof(ClipRect); }
	const void* DataPtr() const { return rects.empty() ? nullptr : rects.data(); }
};

struct ZBucket {
	ClipStack clipStack;
	std::vector<ShapeVertex> shapes;
	std::vector<TextVertex> text;
	std::vector<TextureVertex> textures;
};

struct GuiBatches {
	std::map<uint32_t, ZBucket> zBuckets; // child depth component of Z ordering (integer)

	void Clear() {
		for (auto& [z, bucket] : zBuckets) {
			bucket.clipStack.rects.clear();
			bucket.shapes.clear();
			bucket.text.clear();
			bucket.textures.clear();
		}
		zBuckets.clear();
	}

	bool IsEmpty() const {
		for (const auto& [z, bucket] : zBuckets) {
			if (!bucket.shapes.empty() || !bucket.text.empty() || !bucket.textures.empty()) {
				return false;
			}
		}
		return true;
	}
};

struct RenderTarget {
	VkImageView imageView{ VK_NULL_HANDLE };
	VkFramebuffer framebuffer{ VK_NULL_HANDLE };
	uint32_t width{ 0 };
	uint32_t height{ 0 };
};