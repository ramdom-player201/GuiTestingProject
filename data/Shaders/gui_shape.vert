#version 450

// Static unit quad
layout(location = 0) in vec2 inUnitPos;         // -1.0 to 1.0

// Instance data
layout(location = 1) in vec2 inCentre;
layout(location = 2) in vec4 inBaseColour;      // UNORM unpacked by Vulkan
layout(location = 3) in vec4 inBorderColour;
layout(location = 4) in vec4 inSizeBorder;      // width, height, thickness, unused
layout(location = 5) in vec4 inRadii;           // TL, TR, BR, BL
layout(location = 6) in uint inClipStackOffset; // Index for SSBO
layout(location = 7) in uint inClipCount;       // Number of clips to apply from SSBO

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
layout(location = 5) out vec2 fragCentre;
layout(location = 6) flat out uint fragClipStackOffset;
layout(location = 7) flat out uint fragClipCount;

void main() {
    // Scale unit quad to actual size and translate to centre
    vec2 halfSize = inSizeBorder.xy * 0.5;
    vec2 worldPos = inCentre + (inUnitPos * halfSize);

    // Local position relative to centre (SDF)
    fragLocalPos = inUnitPos * halfSize;

    // Convert screen position to NDC
    vec2 screenPos = worldPos - vec2(viewport.offsetX, viewport.offsetY);
    vec2 ndcPos = (screenPos / vec2(viewport.width, viewport.height)) * 2.0 - 1.0;
    ndcPos.y = -ndcPos.y;

    gl_Position = vec4(ndcPos, 0.0, 1.0);

    // Pass through
    fragBaseColour = inBaseColour;
    fragBorderColour = inBorderColour;
    fragSizeBorder = inSizeBorder;
    fragRadii = inRadii;
    fragCentre = inCentre;
    fragClipStackOffset = inClipStackOffset;
    fragClipCount = inClipCount;
}