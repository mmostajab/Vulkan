#version 430 core

layout(std140, binding = 0) uniform TransformBlock {
    mat4 proj_mat;
    mat4 view_mat;
    mat4 world_mat;
} ;

uniform vec3 position[6] = { vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f) };
uniform vec3 color[3] = { vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f) };

out vec4 vec_color;

void main() {
	gl_Position =  vec4( mat3(view_mat * world_mat) * position[gl_VertexID], 1.0f);
	vec_color = vec4(color[gl_VertexID / 2], 1.0f);
}