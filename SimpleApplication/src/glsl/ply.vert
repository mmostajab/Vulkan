#version 430 core

layout(std140, binding = 0) uniform TransformBlock {
    mat4 proj_mat;
    mat4 view_mat;
    mat4 world_mat;
} ;

layout(std140, binding = 1) uniform LightBlock {
    vec4 dir;
    vec4 pos;
} ;

layout (location = 5) uniform int use_const_color;
layout (location = 6) uniform vec3 const_color = vec3(1.0f, 0.0f, 0.0f);

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 normal;

out VS_OUT {
    vec4 pos;
    vec3 light_dir_view;
    vec3 normal;
    vec4 color;
} vs_out;

void main() {
    vec4 p_view = view_mat * world_mat * vec4(position, 1.0f);
	vs_out.normal = normal;
    vs_out.light_dir_view = mat3(view_mat * world_mat) * dir.xyz;
    vs_out.pos = vec4(position, 1.0);

	if(use_const_color == 0)
		vs_out.color = vec4(normal, 1.0f);
	else
		vs_out.color = vec4(normal, 1.0f);
	
	vec4 randColor;
	if((gl_VertexID / 10) % 2 == 0)
	   randColor = vec4(0, 1, 0, 1);
	else
	   randColor = vec4(1, 0, 0, 1);

    vs_out.color = randColor;//vec4((normalize(normal) + 1.0f) / 2.0f, 1.0f);

    gl_Position   = proj_mat * p_view;
}