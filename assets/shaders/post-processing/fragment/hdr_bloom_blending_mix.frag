#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloom;

uniform float blend_strength;

void main() {
    // Extract scene color and bloom color
    vec3 scene_color = texture(scene, TexCoords).rgb;
    vec3 bloom_color = texture(bloom, TexCoords).rgb;
    
    // Blend both color
    vec3 blen_color = mix(scene_color, bloom_color, blend_strength);
    FragColor = vec4(blen_color, 1.0);
}