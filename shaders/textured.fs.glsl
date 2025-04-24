#version 330

// From vertex shader
in vec3 v_position;
in vec2 texcoord;

// Application data
uniform float normal_strength;
uniform sampler2D color_sampler;
uniform sampler2D normal_sampler;
uniform vec4 fcolor;
uniform uint object_id;

// Outputs
layout (location = 0) out vec4 color;			// color
layout (location = 1) out uint out_object_id;	// object ID
layout (location = 2) out vec3 position;		// world-space position
layout (location = 3) out vec3 normal;			// normals

void main() {
	vec4 texColor = texture(color_sampler, texcoord);
	color = fcolor * texColor;

	position = v_position;

	vec4 normal_color = texture(normal_sampler, texcoord);
	if (normal_strength > 0.0 && normal_color.a > 0.0) {
		// apply normal strength by blending between identity normal
		normal = mix(vec3(0.486, 0.482, 0.894), normal_color.rgb, normal_strength);
	} else {
		// set normal to 0 if this object doesn't use normals
		// not doing so causes the above function to execute anyways on some GPUs
		normal = vec3(0.0, 0.0, 0.0);
	}

	// Alpha mask the object ID fragment
	if (texColor.a > 0.0) {
		out_object_id = object_id;
	} else discard;
}
