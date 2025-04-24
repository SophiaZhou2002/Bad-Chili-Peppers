#version 330

uniform vec3 color;

// Outputs
layout (location = 0) out vec4 out_color;		// color
layout (location = 1) out uint out_object_id;	// object ID

void main()
{
	out_color = vec4(color, 1.0);
	out_object_id = uint(0);
}
