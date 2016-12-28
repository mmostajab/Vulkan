#version 430 core

in VS_OUT {
	vec4 color;
	vec4 world_pos;
} fs_in;

layout (location = 0) uniform float radius;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 normal_depth;

void main() {

    float gradient = exp(-1.0f / (8 * radius) * (fs_in.world_pos.x * fs_in.world_pos.x + fs_in.world_pos.z * fs_in.world_pos.z));
    vec4 color = fs_in.color * gradient;

	out_color = color;
	normal_depth = vec4(0.0f);

}