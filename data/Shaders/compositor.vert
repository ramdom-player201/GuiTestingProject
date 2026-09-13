#version 450

layout(push_constant) uniform PushConstants {
	vec2 offset;  // Normalized X, Y (0.0 to 1.0)
	vec2 scale;   // Normalized Width, Height (0.0 to 1.0)
} pc;

layout(location = 0) out vec2 fragTexCoord;

void main() {
	// Fullscreen triangle strip
	vec2 positions[6] = vec2[](
		vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0),
        vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0)
	);

	vec2 texCoords[6] = vec2[](
		vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0),
        vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0)
	);

	vec2 pos = positions[gl_VertexIndex];

	pos = (pos * pc.scale) + pc.offset;
	pos = pos * 2.0 - 1.0;
	pos.y = -pos.y;

	gl_Position = vec4(pos, 0.0, 1.0);
	fragTexCoord = texCoords[gl_VertexIndex];
}