#version 330

// From vertex shader
in vec2 texcoord;
in vec3 vertColor;

// Application data
uniform sampler2D sampler0;
uniform vec4 fcolor;
uniform float time;
uniform uint object_id;

// Outputs
layout (location = 0) out vec4 color;			// color
layout (location = 1) out uint out_object_id;	// object ID

void main()
{
    // M1 interpolation implementation
    vec3 lerp = mix(vertColor.yzx, vertColor.xyz, pow(cos(time*0.1), 2));
	color = vec4(lerp, 1.0) * fcolor * texture(sampler0, texcoord);

    // Alpha mask the object ID fragment
	if (color.a > 0.0) {
		out_object_id = object_id;
	} else discard;
}
