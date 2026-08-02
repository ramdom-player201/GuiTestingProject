#pragma once

// 2D rectangle for drawing images to screen
struct DrawRect {
	int32_t x{ 0 };
	int32_t y{ 0 };
	uint32_t width{ 0 };
	uint32_t height{ 0 };
};

// Used to define regions to render viewport
//struct ViewportLayoutRect {
//	uint32_t panelId;
//	DrawRect rect;
//};