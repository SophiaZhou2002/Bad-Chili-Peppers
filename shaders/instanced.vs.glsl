#version 330 core

// Vertex attributes
layout(location = 0) in vec3 aPos;        
layout(location = 1) in vec2 aTexCoord;

// Instance attributes (mat3 split into 3 vec3s)
layout(location = 2) in vec3 instance_transform_0;
layout(location = 3) in vec3 instance_transform_1;
layout(location = 4) in vec3 instance_transform_2;
layout(location = 5) in float instance_alpha;

// Outputs
out vec2 texcoord;
out float alpha;

uniform mat3 projection;

void main() {
    // Reconstruct mat3 from vec3 inputs
    mat3 instance_transform = mat3(
        instance_transform_0,
        instance_transform_1,
        instance_transform_2
    );

    // Apply transformations
    vec3 worldPos = instance_transform * vec3(aPos.xy, 1.0);
    vec3 projectedPos = projection * worldPos;

    // Convert vec3 -> vec4 (OpenGL requires vec4 for gl_Position)
    gl_Position = vec4(projectedPos.xy, 0.0, 1.0);
    
    texcoord = aTexCoord;
    alpha = instance_alpha;
}
