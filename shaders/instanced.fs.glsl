#version 330 core

in vec2 texcoord;
in float alpha;

uniform sampler2D myTexture;

// Outputs
layout (location = 0) out vec4 color;			// color
layout (location = 1) out uint out_object_id;	// object ID

void main() {
	vec4 texColor = texture(myTexture, texcoord);
	color = vec4(texColor.rgb, texColor.a * alpha); // Apply alpha fade
	// Alpha mask the object ID fragment
	if (color.a > 0.0) {
		out_object_id = uint(2);
	} else discard;
}
