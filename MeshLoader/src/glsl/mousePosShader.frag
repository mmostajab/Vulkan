#version 430 core
   
layout(std140, binding = 0) uniform TransformBlock {
    mat4 proj_mat;
    mat4 view_mat;
    mat4 world_mat;
};

layout(location = 0) uniform float radius = 0.03;

layout(location = 0) in vec4 viewPos;
layout(location = 1) in vec4 viewCenter;

uniform vec3 lightPos ;

void main()
{

	vec3 distanceCenter = viewPos.xyz - viewCenter.xyz;

	vec3 dir = normalize(viewPos.xyz);
	
	float a = 1.0f;
	float bPrime = dot(-viewCenter.xyz, dir);
	float c = dot(viewCenter.xyz, viewCenter.xyz) - radius * radius;

	float deltaPrime  = bPrime * bPrime - a * c;

	if(deltaPrime >= 0){
		float t = -bPrime - sqrt(deltaPrime);
		vec3 intersect_point = t * dir;
		vec3 viewNormal = normalize(intersect_point - viewCenter.xyz);

		vec3 lightPosView = vec3(view_mat * vec4(lightPos, 1.0f));
		vec3 lightDir = normalize(vec3(1, 1, 1));// normalize(lightPosView - intersect_point);

		//gl_FragColor = (vec4(viewNormal, 1.0) + vec4(1.0)) / 2.0;
		gl_FragColor = vec4(dot(lightDir, viewNormal) * vec3(0.2, 0.7, 0.6), 1.0);

		vec4 projected_intersection_point = proj_mat * vec4(intersect_point, 1.0);
		gl_FragDepth = (projected_intersection_point.z  / projected_intersection_point.w + 1.0f) / 2.0f;
	} 
	else discard;	
}