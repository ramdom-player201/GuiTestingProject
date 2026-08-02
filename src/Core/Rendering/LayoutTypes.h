#pragma once

// 2D rectangle for drawing images to screen, must cast to int32_t and uint32_t when passed to VkRect2D
struct DrawRect {
	float x{ 0 };
	float y{ 0 };
	float width{ 0 };
	float height{ 0 };
};

struct Vec2 {
	float x{ 0.0f };
	float y{ 0.0f };
	Vec2(float inX = 0.0f, float inY = 0.0f) :x(inX), y(inY) {} // constructor
};

// A single axis value unioning a ratio and absolute value
struct UDim {
	float scale{ 0.0f }; // 0.0 to 1.0 (ratio of parent size
	float offset{ 0.0f }; // Absolute pixels, but multiplied by scale

	UDim(float inScale = 0.0f, float inOffset = 0.0f) :scale(inScale), offset(inOffset) {} // Constructor
};

// 2D coordinate or size
struct UDim2 {
	UDim x;
	UDim y;

	UDim2(float scaleX = 0.0f, float offsetX = 0.0f, float scaleY = 0.0f, float offsetY = 0.0f) :
		x(scaleX, offsetX), y(scaleY, offsetY) {} // Constructor from raw values
	UDim2(UDim inX, UDim inY) :x(inX), y(inY) {} // Constructor from UDim pair
};

// Input structs
struct InputEventResult {
	bool inputConsumed{ false }; // Is the input consumed by the gui, or does it need to be passed to a viewport?
	uint32_t targetViewportId{ UINT32_MAX }; // Max acts as null value (no viewport)
	// Mouse coordinates relative to the viewport instead of screen (only valid if viewport)
	float localX{ 0.0f };
	float localY{ 0.0f };
};