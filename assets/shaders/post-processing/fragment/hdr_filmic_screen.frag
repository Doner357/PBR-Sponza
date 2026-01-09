#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

// Used for gamma correction
layout (std140) uniform GammaCorrection {
	float gamma;    // 4 bytes
};

vec3 filmic(vec3 x) {
    vec3 X = max(vec3(0.0), x - 0.004);
    vec3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
    return result;
}

void main() {
    // Get the hdr color from screen texture
    vec3 hdr_color = texture(screenTexture, TexCoords).rgb;

    // Do exposure tone mapping
    vec3 mapped = filmic(hdr_color);

    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
}