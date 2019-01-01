#version 430 core

layout(std140, set=0, binding=0) uniform Transformations {
    mat4 projMatrix;
    mat4 viewMatrix;
    mat4 worldMatrix;
} ;

layout(location = 0) in vec4 worldPos;
layout(location = 1) in vec3 worldNormal;

layout(location = 0) out vec4 outColor;

void main(){
	vec3 diffuse_albedo  = vec3(1.0f, 1.0f, 1.0f);
	vec3 specular_albedo = vec3(0.7f);
	float specular_power = 128.0f; 

	float shading_level = 1.0;

	vec4 color = vec4(0.4f, 0.7f, 0.5f, 1.0f);

	vec4 v = normalize(viewMatrix * worldPos);
	vec3 l = normalize(vec3(1.0f, 1.0f, 1.0f));
	vec3 n = normalize(mat3(viewMatrix) * worldNormal);
	vec3 r = reflect(-l, n);
	vec3 h = normalize(l + v.xyz);
	
	vec3 ambient_factor  = vec3(0.20f, 0.20f, 0.20f);
	vec3 diffuse_factor  = diffuse_albedo * max(dot(n,l), 0.0f);
	vec3 specular_factor = specular_albedo * pow(max(dot(r, v.xyz), 0.0f), specular_power); 

	outColor = color * mix(vec4(0.0), vec4( ambient_factor + diffuse_factor + specular_factor, 1.0), shading_level);
}