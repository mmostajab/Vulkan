#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform ModelViewProjUBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
} mvpUBO;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main()
{
    gl_Position = mvpUBO.proj * mvpUBO.view * mvpUBO.model * vec4(inPosition.x, 0.0f, inPosition.y, 1.0f);
    fragColor = inColor;
}