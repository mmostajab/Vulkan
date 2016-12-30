#version 430 core

layout(std140, set=0, binding=0) uniform Transformations {
    mat4 projMatrix;
    mat4 viewMatrix;
    mat4 worldMatrix;
} ;

layout( location = 0 ) in vec3 pos;
layout( location = 1 ) in vec3 normal;

layout(location = 0) out vec4 worldPos;
layout(location = 1) out vec3 worldNormal;

void main(){
    worldPos    = worldMatrix * vec4(pos, 1.0f);
    worldNormal = normalize( mat3(worldMatrix) * normal ); 
    gl_Position = projMatrix * viewMatrix * worldPos;
}