#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

// Used for gamma correction
layout (std140) uniform GammaCorrection {
	float gamma;    // 4 bytes
};

// Used to control the exposure scale
uniform float attenuation;

void main() {
    // Get the hdr color from screen texture
    vec3 hdr_color = texture(screenTexture, TexCoords).rgb;

    // Do exposure tone mapping
    vec3 mapped = hdr_color * attenuation;

    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
}