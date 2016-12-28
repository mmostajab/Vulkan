#version 430 core

in VS_OUT {
    vec4 viewPos;
    vec3 viewNormal;
} fs_in;

layout(location = 0) out vec4 outColor;

void main(){
	outColor = vec4(fs_in.viewNormal, 1.0);
}