#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;
out float scale;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform float u_scale;

void main() {
	texcoord = in_texcoord;
    
    mat3 transform_scaled = transform;
    scale = u_scale;
    transform_scaled[0][0] *= scale;
    transform_scaled[1][1] *= scale;

	vec3 pos = projection * transform_scaled * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}