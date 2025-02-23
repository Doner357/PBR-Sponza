#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

// Used for gamma correction
layout (std140) uniform GammaCorrection {
	float gamma;    // 4 bytes
};

void main() {
    // Get the hdr color from screen texture
    vec3 hdr_color = texture(screenTexture, TexCoords).rgb;

    // Do Reinhard tone mapping
    vec3 mapped = hdr_color / (hdr_color + vec3(1.0));

    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / gamma));

    mapped = clamp(mapped, 0.0, 1.0);

    FragColor = vec4(mapped, 1.0);
}