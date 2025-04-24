#version 330

// From vertex shader
in vec2 texcoord;
in float scale;
// in float intensity;

// Application data
uniform sampler2D sampler0;
uniform vec4 fcolor;
uniform uint object_id;

// Outputs
layout (location = 0) out vec4 color;			// color
// layout (location = 1) out uint out_object_id;	// object ID
layout (location = 2) out float fire_radius;	// fire lighting radius

// Larger scale -> smaller visual size (due to how UVs work)
vec2 scale_uvs(vec2 uv, float scale) {
    return (uv*scale)-(scale-1.0)/2.0;
}

void main() {
    // Preserve visual size of sprite
	vec4 texColor = texture(sampler0, scale_uvs(texcoord, scale));
	color = fcolor * texColor;

    vec2 diff = texcoord - vec2(0.5, 0.5);
    fire_radius = clamp((1.0-length(diff)-0.75)*1.3, 0.0, 1.0);

	// Alpha mask the object ID fragment
	// if (texColor.a > 0.0) {
	// 	out_object_id = object_id;
	// }
}