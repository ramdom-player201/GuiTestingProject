#version 450

// Input from vertex
layout(location = 0) in vec2 fragLocalPos;
layout(location = 1) in vec4 fragBaseColour;
layout(location = 2) in vec4 fragBorderColour;
layout(location = 3) in vec4 fragSizeBorder;
layout(location = 4) in vec4 fragRadii;
layout(location = 5) in vec2 fragCentre;
layout(location = 6) flat in uint fragClipStackOffset;
layout(location = 7) flat in uint fragClipCount;

// SSBO: Clip rect buffer
struct ClipRect{
    vec2 pos;
    vec2 halfSize;
    vec4 radii;
};

layout(std430, set = 0, binding = 0) readonly buffer ClipData{
    ClipRect clips[];
} clipData;

layout(location = 0) out vec4 outColor;

// SDF for rounded rectangle
float sdRoundedBox(vec2 p, vec2 halfSize, vec4 radii) {

    // Select radius for quadrant (Screen space Y>0 is down)
    float r;
    if (p.x < 0.0 && p.y < 0.0)      r = radii.x; // Top-Left
    else if (p.x > 0.0 && p.y < 0.0) r = radii.y; // Top-Right
    else if (p.x > 0.0 && p.y > 0.0) r = radii.z; // Bottom-Right
    else                             r = radii.w; // Bottom-Left

    // Fit corners to halfSize boundary
    vec2 d = abs(p) - halfSize + r;

    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - r;
}

// Test if point is inside a clip rect
float clipSDF(vec2 pos, ClipRect clip) {
    return sdRoundedBox(pos - clip.pos, clip.halfSize, clip.radii);
}

void main() {
    // Clip test (reconstruct absolute screen position)
    vec2 screenPos = fragLocalPos + fragCentre;
    for (uint i = 0; i < fragClipCount; i++) {
        if (clipSDF(screenPos, clipData.clips[fragClipStackOffset + i]) > 0.0) {
            discard; // early exit so redundant calculations can be skipped
        }
    }

    vec2 halfSize = fragSizeBorder.xy * 0.5;
    float borderThickness = fragSizeBorder.z;

    // Calculate single outer shape SDF
    float shapeDist = sdRoundedBox(fragLocalPos, halfSize, fragRadii);

    // Calculate anti-aliasing smoothness based on pixel density
    float outerSoftness = fwidth(shapeDist) * 1.5;
    float innerSoftness = fwidth(shapeDist + borderThickness) * 1.5;

    // Calculate masks based on distance
    // > Outer mask (fade along edge)
    float outerMask = 1.0 - smoothstep(-outerSoftness, outerSoftness, shapeDist);

    // > Inner mask (fade along border inner edge)
    float innerMask = 1.0 - smoothstep(-innerSoftness, innerSoftness, shapeDist + borderThickness);

    // Calculate coverage
    float fillCoverage = innerMask;
    float borderCoverage = outerMask - innerMask;

    // Combine colours with pre-multiplied alpha
    vec3 finalColour = (fragBaseColour.rgb * fragBaseColour.a * fillCoverage) +
                       (fragBorderColour.rgb * fragBorderColour.a * borderCoverage);

    float finalAlpha = (fragBaseColour.a * fillCoverage) + (fragBorderColour.a * borderCoverage);

    outColor = vec4(finalColour, finalAlpha);
}