#version 330

uniform sampler2D screen_color_texture;
uniform sampler2D limited_vision_object_texture;
uniform usampler2D screen_object_id_texture;
uniform sampler2D screen_fire_radius_texture;
uniform sampler2D screen_position_texture;
uniform sampler2D screen_normal_texture;
uniform bool limited_vision;
uniform vec2 player_world_position;
uniform vec2 player_position;
uniform vec3 shadow_color;

in vec2 texcoord;

layout(location = 0) out vec4 color;

// const int bayer2x2[4] = int[4](
// 	0, 2,
// 	3, 1
// );
// const int bayer4x4[16] = int[16](
// 	 0,  8,  2, 10,
// 	12,  4, 14,  6,
// 	 3, 11,  1,  9,
// 	15,  7, 13,  5
// );
// const int bayer8x8[64] = int[64](
// 	 0, 32,  8, 40,  2, 34, 10, 42, 
// 	48, 16, 56, 24, 50, 18, 58, 26,
// 	12, 44,  4, 36, 14, 46,  6, 38,
// 	60, 28, 52, 20, 62, 30, 54, 22,
// 	 3, 35, 11, 43,  1, 33,  9, 41,
// 	51, 19, 59, 27, 49, 17, 57, 25,
// 	15, 47,  7, 39, 13, 45,  5, 37,
// 	63, 31, 55, 23, 61, 29, 53, 21
// );

void main() {
	ivec2 screen_size = textureSize(screen_color_texture, 0);
	float aspect_ratio = float(screen_size.x)/screen_size.y;
	
    vec4 base_color = texture(screen_color_texture, texcoord);
    vec4 object_color = texture(limited_vision_object_texture, texcoord);
    // uint object_id = texture(screen_object_id_texture, texcoord).r;
	vec3 position = texture(screen_position_texture, texcoord).rgb;
	vec3 normal = texture(screen_normal_texture, texcoord).rgb;
	vec4 lighting = vec4(vec3(0.0), 1.0);
	
	// if the normal texture is not empty at this fragment, apply normal computations
	if (dot(normal, normal) > 0.0) {
		normal = normalize(normal*2.0-1.0); // remap normal textures between -1.0 to 1.0
		// player is the light source
		// offset on the y to make vertical navigation affect the lighting less
		// offset on the z to make the light more softly spread out
		vec3 light_position = vec3(player_world_position.x, player_world_position.y-20.0, 40.0);
		vec3 light_dir = normalize(light_position - position);
		// standard diffuse shading
		float n_dot_l = max(dot(normal, light_dir), 0.0);
		vec3 light_color = vec3(1.0, 1.0, 0.5); // hardcoded slightly yellow light
		lighting = n_dot_l * 0.5 * vec4(light_color, 1.0); // hardcoded light intensity 0.5
	}

	if (limited_vision) {
		float pixel_resolution = screen_size.y/1.0; // Larger denominator -> larger pixels
		const int dim = 8; // Bayer matrix dimension to use
		const float dim_sq = float(dim*dim);

		vec2 texcoord_scaled = vec2(texcoord.x*aspect_ratio, texcoord.y);
		// ivec2 texcoord_quantized = ivec2(texcoord_scaled*pixel_resolution);
		
		// Difference from player
		vec2 diff_from_player = texcoord_scaled - vec2(player_position.x*aspect_ratio, player_position.y);
		float gradient = length(diff_from_player); // Squared, quantized distance from player
		gradient = smoothstep(0.1, 0.4, gradient); // Remapping values, TODO add randomness to simulate fire

		float fire_radius = texture(screen_fire_radius_texture, texcoord).r;
		// Invert the fire radius (so that it is compatible with the computed gradient) and multiply
		gradient *= 1.0-fire_radius;
		gradient = clamp(gradient, 0.0, 1.0);

		// Quantize strength to appropriate number of bands (makes pattern radially symmetrical)
		// gradient = int(gradient*dim*2.0)/(dim*2.0);

		// Compute Bayer dithering pattern value at current pixel
		// int i = texcoord_quantized.x%dim;
		// int j = (texcoord_quantized.y%dim)*dim;
		// int bayer_val = 0;
		// if 		(dim == 2) bayer_val = bayer2x2[i+j];
		// else if (dim == 4) bayer_val = bayer4x4[i+j];
		// else if (dim == 8) bayer_val = bayer8x8[i+j];
		
		// Binarize gradient based on Bayer dithering pattern
		// if (gradient > bayer_val/dim_sq) gradient = 1.0;
		// else gradient = 0.0;

		// Blend limited vision objects with base render
		if (object_color.a > 0.0) {
			vec4 alpha_blended_base_color = mix(base_color, object_color, object_color.a);
			base_color = mix(alpha_blended_base_color, base_color, pow(gradient, 4));
		}
		base_color = mix(base_color+lighting, base_color, pow(gradient, 4));

		// Add a bluish tint as the gradient gets darker, and darken the blended image as well
		vec3 darken = mix(base_color.rgb, shadow_color, gradient*0.4)*(1.0-gradient*0.4);
		color = vec4(darken, 1.0);

		// Visualize object IDs
		// if (object_id > uint(0)) color = vec4(vec3(255.0/object_id/255.0), 1.0);
		// else color = vec4(vec3(0.0), 1.0);
	} else {
		// If current fragment is not occupied by a Fire block, overwrite it by limited vision objects
		// if (object_id != uint(1)) {
		base_color = mix(base_color, object_color, object_color.a);
		// }
		color = base_color;
	}
}