#version 430 core
#extension GL_EXT_geometry_shader4 : enable

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;
   
layout(location = 0) uniform float radius = 0.03;

layout(std140, binding = 0) uniform TransformBlock {
    mat4 proj_mat;
    mat4 view_mat;
    mat4 world_mat;
};

layout(location = 0) out vec4 viewPos;
layout(location = 1) out vec4 viewCenter;

void main()
{
    vec4 _viewcenter = gl_PositionIn[0];

     // ?
    vec4 viewX = vec4(1.4, 0, 0, 0);
    vec4 viewY = vec4(0, 1.4, 0, 0);

	viewCenter = _viewcenter;
	viewPos    = viewCenter + 1.1*radius * ( -viewX -viewY);
    gl_Position = proj_mat * viewPos;
    EmitVertex();

    viewCenter = _viewcenter;
	viewPos    = viewCenter + 1.1*radius * ( +viewX -viewY);
    gl_Position = proj_mat * viewPos;
    EmitVertex();

    viewCenter = _viewcenter;
	viewPos    = viewCenter + 1.1*radius * ( -viewX +viewY);
    gl_Position = proj_mat * viewPos;
    EmitVertex();

    viewCenter = _viewcenter;
	viewPos    = viewCenter + 1.1*radius * ( +viewX +viewY);
    gl_Position = proj_mat * viewPos;
    EmitVertex();

    EndPrimitive();

}