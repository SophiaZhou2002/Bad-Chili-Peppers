#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec3 v_position;
out vec2 texcoord;
out vec3 tempColor;

// Application data
uniform mat3 transform;
uniform mat3 projection;

void main()
{
	vec3 world_pos = transform * vec3(in_position.xy, 1.0);
	vec3 pos = projection * world_pos;
	gl_Position = vec4(pos.xy, in_position.z, 1.0);

	v_position = world_pos;
	texcoord = in_texcoord;
}