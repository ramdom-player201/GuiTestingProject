#version 450

// Input from vertex
layout(location = 0) in vec2 fragLocalPos;
layout(location = 1) in vec4 fragBaseColour;
layout(location = 2) in vec4 fragBorderColour;
layout(location = 3) in vec4 fragSizeBorder;
layout(location = 4) in vec4 fragRadii;

// Push constants: clip stack
struct ClipRect {
    vec2 pos;
    vec2 halfSize;
    vec4 radii;
};

layout(push_constant, std430) uniform FragPush {
    layout(offset = 16) ClipRect clips[3];
    int clipCount;
    int _pad0;
    int _pad1;
    int _pad2;
};

layout(location = 0) out vec4 outColor;

// SDF for rounded rectangle
float sdRoundedBox(vec2 p, vec2 halfSize, vec4 radii) {
    vec2 d = abs(p) - halfSize;

    float r;
    if (p.x < 0.0 && p.y > 0.0)      r = radii.x; // Top-Left
    else if (p.x > 0.0 && p.y > 0.0) r = radii.y; // Top-Right
    else if (p.x > 0.0 && p.y < 0.0) r = radii.z; // Bottom-Right
    else                               r = radii.w; // Bottom-Left

    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - r;
}

// Test if point is inside a clip rect
float clipSDF(vec2 pos, ClipRect clip) {
    return sdRoundedBox(pos - clip.pos, clip.halfSize, clip.radii);
}

void main() {
    vec2 halfSize = fragSizeBorder.xy * 0.5;
    float borderThickness = fragSizeBorder.z;

    // Fill SDF
    float fillDist = sdRoundedBox(fragLocalPos, halfSize - borderThickness, fragRadii - borderThickness);

    // Outline SDF
    float outlineDist = sdRoundedBox(fragLocalPos, halfSize, fragRadii);

    // Determine colour
    vec4 colour;
    if (fillDist <= 0.0) {
        colour = fragBaseColour;
    } else if (outlineDist <= 0.0) {
        colour = fragBorderColour;
    } else {
        discard;
    }

    // Anti-aliasing
    float edgeSoftness = fwidth(outlineDist) * 1.5;
    float alpha = 1.0 - smoothstep(-edgeSoftness, edgeSoftness, outlineDist);

    // Clip test
    for (int i = 0; i < clipCount; i++) {
        if (clipSDF(fragLocalPos, clips[i]) > 0.0) {
            discard;
        }
    }

    outColor = vec4(colour.rgb, colour.a * alpha);
}