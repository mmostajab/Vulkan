#version 430 core

layout(std140, set=0, binding=0) uniform Transformations {
    mat4 projMatrix;
    mat4 viewMatrix;
    mat4 worldMatrix;
} ;

layout( location = 0 ) in vec3 pos;
layout( location = 1 ) in vec3 normal;

out VS_OUT {
    vec4 pos;
    vec3 normal;
} vs_out;

void main(){
    mat4 mv = viewMatrix * worldMatrix;
    vs_out.pos = mv * vec4(pos, 1.0f);
    vs_out.normal = (mv * vec4(normal, 0.0f)).xyz; 
    gl_Position = projMatrix * vs_out.pos;
}