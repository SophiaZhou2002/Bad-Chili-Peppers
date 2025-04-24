#version 330

// From Vertex Shader
in vec3 vcolor;
in vec2 vpos; // Distance from local origin

// Application data
// uniform sampler2D sampler0;
// uniform vec3 fcolor;
// uniform int light_up;

// Outputs
layout (location = 0) out vec4 color;			// color
layout (location = 1) out uint out_object_id;	// object ID

void main()
{
	color = vec4(1.0, 1.0, 1.0, 1.0);
	out_object_id = uint(0);
}