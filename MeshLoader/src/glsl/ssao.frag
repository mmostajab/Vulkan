#version 430 core

// #define MY_DEBUG

// Samplers for pre-rendered color, normal and depth
layout (binding = 0) uniform sampler2D sColor;
layout (binding = 1) uniform sampler2D sNormalDepth;

layout (location = 0) uniform int ssao_active = 1;

// Final output
layout (location = 0) out vec4 color;

// Various uniforms controling SSAO effect
uniform int rendering_state;
uniform float ssao_level      = 1.0;
uniform float ssao_radius     = 0.2;
uniform uint point_count      = 32;
uniform bool randomize_points = true;

// Uniform block containing up to 256 random directions (x,y,z,0)
// and 256 more completely random vectors
layout (binding = 2, std140) uniform SAMPLE_POINTS {
    vec4 pos[256];
    vec4 random_vectors[256];
} points;

void main(void) {

	// Get texture position from gl_FragCoord
    vec2 P = gl_FragCoord.xy / textureSize(sNormalDepth, 0);

	//if(ssao_active == 0 || length(textureLod(sNormalDepth, P, 0).xyz) == 0){ 
	 // color = textureLod(sNormalDepth, P, 0);
	 // color = vec4(color.w, color.w, color.w, 1.0);
		//color = textureLod(sColor, P, 0);
		//color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	//	return;
	//}

	// ND = normal and depth
    vec4 ND = textureLod(sNormalDepth, P, 0);
    
	// Extract normal and depth
    vec3 N = ND.xyz;
    float my_depth = ND.w;

    // Local temporary variables
    int i, j, n;

    float occ = 0.0;
    float total = 0.0;

    // n is a pseudo-random number generated from fragment coordinate
    // and depth
    n = (int(gl_FragCoord.x * 7123.2315 + 125.232) *
         int(gl_FragCoord.y * 3137.1519 + 234.8)) ^
         int(my_depth);
    
	// Pull one of the random vectors
    vec4 v = points.random_vectors[n & 255];
	
    // For each random point (or direction)...
    for (i = 0; i < point_count; i++){
    
	    // Get direction
        vec3 dir = points.pos[i].xyz;

        // Put it into the correct hemisphere
        if (dot(N, dir) < 0.0f)
            dir = -dir;

		// f is the distance we've stepped in this direction
        // z is the interpolated depth
        float f = 0.0f;
        float z = my_depth;

        // We're going to take 4 steps - we could make this
        // configurable
        total += 16.0f;
		float r = ssao_radius / 16.0f;

        for (j = 0; j < 16; j++){
            // Step in the right direction
            f += r;

            // Step _towards_ viewer reduces z
            z -= dir.z * r;

            // Read depth from current fragment
            float their_depth = textureLod(sNormalDepth, (P + dir.xy * f), 0).w;

            // If we're obscured, accumulate occlusion
            if ((z - their_depth) > 0.0){

				float d = length(f * dir.xy);

				float h = atan(abs(z - my_depth) / d);

				vec3 tmp = cross(N, vec3(dir.xy, 0.0));
				vec3 T   = normalize(cross(tmp, N));
				float f_prime = (f * dir.x - my_depth) / T.x;
				T = f_prime * T;


				float t = atan(abs(T.z - my_depth) / length(T.xy));

				//occ += (sin(h) - sin(t));

				occ += clamp(4.0 * dot(N, dir), 0.0f, 1.0f);
				//occ += 1.0f;
            }
        }
    }

    // Calculate occlusion amount
    float ao_amount = 1.0 - occ / total;

    // Get object color from color texture
    vec4 object_color =  textureLod(sColor, P, 0);

    // Mix in ambient color scaled by SSAO level

	if(rendering_state == 0)
		color = object_color * mix(vec4(0.1), vec4(ao_amount), ssao_level);
	else if(rendering_state == 1)
		color = vec4(ao_amount);
	else if(rendering_state == 2)
		color = object_color;
	//else if(rendering_state == 3)
	//color = vec4(ao_amount);
}