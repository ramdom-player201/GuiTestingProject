#pragma once

#include "UiFrame.h"

// UiShape can draw an arbritary rectangle with rounded edges

// Packs RGBA8 into uint32_t for the GPU
inline uint32_t PackColour(ColourRGBA_8 c) {
	return (static_cast<uint32_t>(c.a) << 24) |
		(static_cast<uint32_t>(c.b) << 16) |
		(static_cast<uint32_t>(c.g) << 8) |
		static_cast<uint32_t>(c.r);
};

class UiShape : public UiFrame {
protected:
	Vec4 cornerRadii{ 0.0f,0.0f,0.0f,0.0f }; // TL, TR, BR, BL
	float borderThickness{ 0.0f };
	ColourRGBA_8 baseColour{ 255,255,255,255 }; // White, opaque
	ColourRGBA_8 borderColour{ 0,0,0,255 }; // Black, opaque

	// Transparency not stored, but written into colour's RGBA via Getter/Setter

	ShapeVertex cachedVertices[4];

	std::optional<ClipRect> GetClipRect() const override;
	void DrawElement(UiPassParams& params, bool needsRedraw) override;
public:
	UiShape();
};


















// AI generated shader code for UiShape
/*
#version 450

// Input from Vertex Shader
layout(location = 0) in vec2 fragLocalPos;
layout(location = 1) in vec4 fragBaseColor;
layout(location = 2) in vec4 fragBorderColor;
layout(location = 3) in vec4 fragSizeAndBorder;
layout(location = 4) in vec4 fragRadii;

layout(location = 0) out vec4 outColor;

// The core math: Signed Distance Field for a box with per-corner radii
float sdPerCornerBox(vec2 p, vec2 halfSize, vec4 radii) {
	// p is position relative to center. halfSize is half width/height.
	// radii are {TL, TR, BR, BL}

	vec2 d = abs(p) - halfSize;

	// Determine which corner we are in based on the sign of X and Y
	float r;
	if (p.x < 0.0 && p.y > 0.0) r = radii.x; // Top-Left
	else if (p.x > 0.0 && p.y > 0.0) r = radii.y; // Top-Right
	else if (p.x > 0.0 && p.y < 0.0) r = radii.z; // Bottom-Right
	else r = radii.w; // Bottom-Left

	// Standard box SDF formula, substituting the dynamic radius
	return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - r;
}

void main() {
	vec2 halfSize = fragSizeAndBorder.xy * 0.5;
	float borderThickness = fragSizeAndBorder.z;

	// 1. Calculate distance for the FILL area (slightly smaller box)
	float fillDist = sdPerCornerBox(fragLocalPos, halfSize - borderThickness, fragRadii - borderThickness);

	// 2. Calculate distance for the OUTLINE area (the full bounding box)
	float outlineDist = sdPerCornerBox(fragLocalPos, halfSize, fragRadii);

	vec4 finalColor = vec4(0.0);

	// 3. Determine what part of the shape this pixel belongs to
	if (fillDist <= 0.0) {
		finalColor = fragBaseColor; // Inside the fill
	}
	else if (outlineDist <= 0.0) {
		finalColor = fragBorderColor; // Inside the border (between fill and outer edge)
	}
	else {
		discard; // Outside the shape entirely. Tell Vulkan to throw this pixel away.
	}

	// 4. Anti-aliasing (Smooth the edges so they aren't jagged)
	// fwidth() calculates how fast the distance is changing per pixel.
	// We use it to create a smooth fade over 1 pixel.
	float edgeSoftness = fwidth(outlineDist) * 1.5; // 1.5 is a tweakable softness factor
	float alpha = 1.0 - smoothstep(-edgeSoftness, edgeSoftness, outlineDist);

	// Multiply the calculated color by the anti-aliased alpha
	outColor = vec4(finalColor.rgb, finalColor.a * alpha);
}
*/