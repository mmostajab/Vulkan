#version 430 core

layout(std140, binding = 1) uniform LightBlock {
    vec4 dir;
    vec4 pos;
} ;

layout( location = 0 ) uniform vec3 color = vec3(1.0f, 1.0f, 1.0f);

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 normal_depth;

void main() {
	out_color = vec4(color, 1.0f);
	normal_depth = vec4(0.0f, 0.0f, 0.0f, 0.0f);
}