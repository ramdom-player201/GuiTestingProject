#version 450

layout(location = 0) in vec2 inPosition; // 0.0 to 1.0
layout(location = 1) in vec2 inTexCoord;

layout(push_constant) uniform PushConstants {
	vec2 offset;  // Normalized X, Y (0.0 to 1.0)
	vec2 scale;   // Normalized Width, Height (0.0 to 1.0)
} pc;

layout(location = 0) out vec2 fragTexCoord;

void main() {
	// Scale the 0-1 quad to the desired size, then shift by offset
	vec2 pos = (inPosition * pc.scale) + pc.offset;

	// Convert 0-1 space to -1 to 1 NDC space
	pos = pos * 2.0 - 1.0;
	pos.y = -pos.y; // Flip Y because Vulkan NDC is bottom-up, but UI coords are top-down

	gl_Position = vec4(pos, 0.0, 1.0);
	fragTexCoord = inTexCoord;
}