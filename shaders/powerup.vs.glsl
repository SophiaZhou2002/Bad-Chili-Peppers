#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;
in vec3 in_color;

// Passed to fragment shader
out vec2 texcoord;
out vec3 vertColor;

// Application data
uniform mat3 transform;
uniform mat3 projection;

void main()
{
	texcoord = in_texcoord;
    vertColor = in_color;
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}