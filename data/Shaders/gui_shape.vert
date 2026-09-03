#version 450

// Input: ShapeVertex
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inCenter;
layout(location = 2) in vec4 inBaseColour;      // UNORM unpacked by Vulkan
layout(location = 3) in vec4 inBorderColour;
layout(location = 4) in vec4 inSizeBorder;       // width, height, thickness, unused
layout(location = 5) in vec4 inRadii;           // TL, TR, BR, BL

// Push constants: viewport transform
layout(push_constant, std430) uniform VertPush {
    float width;
    float height;
    float offsetX;
    float offsetY;
} viewport;

// Output to fragment
layout(location = 0) out vec2 fragLocalPos;
layout(location = 1) out vec4 fragBaseColour;
layout(location = 2) out vec4 fragBorderColour;
layout(location = 3) out vec4 fragSizeBorder;
layout(location = 4) out vec4 fragRadii;

void main() {
    // Local position relative to center (for SDF)
    fragLocalPos = inPos - inCenter;

    // Convert screen position to NDC
    vec2 screenPos = inPos - vec2(viewport.offsetX, viewport.offsetY);
    vec2 ndcPos = (screenPos / vec2(viewport.width, viewport.height)) * 2.0 - 1.0;
    ndcPos.y = -ndcPos.y;  // Vulkan Y-axis is inverted

    gl_Position = vec4(ndcPos, 0.0, 1.0);

    // Pass through
    fragBaseColour = inBaseColour;
    fragBorderColour = inBorderColour;
    fragSizeBorder = inSizeBorder;
    fragRadii = inRadii;
}