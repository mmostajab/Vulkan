#version 430 core

layout(std140, binding = 0) uniform TransformBlock {
    mat4 proj_mat;
    mat4 view_mat;
    mat4 world_mat;
} ;

layout(std140, binding = 3) uniform GeneralBlock {
    float color_multiplier;
	vec4 color_background;
} ;

in VS_OUT {
	vec4 pos;
	vec3 light_dir_view;
	vec3 normal;
	vec4 color;
} fs_in;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 normal_depth;

uniform vec3 diffuse_albedo  = vec3(1.0f, 1.0f, 1.0f);
uniform vec3 specular_albedo = vec3(0.7f);
uniform float specular_power = 128.0f; 

uniform float shading_level = 1.0;

void main() {
	vec3 v = normalize(view_mat * world_mat * fs_in.pos).xyz;
	vec3 l = normalize(vec3(1, 1, 1));  //! toward the camera
	vec3 n = normalize(mat3(view_mat) * mat3(world_mat) * fs_in.normal);
	vec3 r = reflect(-l, n);
	vec3 h = normalize(l + v);
	
	vec3 ambient_factor  = vec3(0.20f, 0.20f, 0.20f);
	vec3 diffuse_factor  = diffuse_albedo * max(dot(n,l), 0.0f);
	vec3 specular_factor = specular_albedo * pow(max(dot(r, v), 0.0f), specular_power); 

	out_color = fs_in.color * mix(vec4(0.0), vec4( ambient_factor + diffuse_factor + specular_factor, 1.0), shading_level);
	normal_depth = vec4(n, -(view_mat * world_mat * fs_in.pos).z);
}