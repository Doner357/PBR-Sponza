#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

// Used for gamma correction
layout (std140) uniform GammaCorrection {
    float gamma;    // 4 bytes
};

// Based on http://www.oscars.org/science-technology/sci-tech-projects/aces
vec3 aces_tonemap(vec3 color){
	mat3 m1 = mat3(
        0.59719, 0.07600, 0.02840,
        0.35458, 0.90834, 0.13383,
        0.04823, 0.01566, 0.83777
	);
	mat3 m2 = mat3(
        1.60475, -0.10208, -0.00327,
        -0.53108,  1.10813, -0.07276,
        -0.07367, -0.00605,  1.07602
	);
	vec3 v = m1 * color;    
	vec3 a = v * (v + 0.0245786) - 0.000090537;
	vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;

    // Gamma correction
    float safe_gamma = max(gamma, 0.01);
	return pow(clamp(m2 * (a / b), 0.0, 1.0), vec3(1.0 / safe_gamma));
}

void main() {
    // Get the hdr color from screen texture
    vec3 hdr_color = texture(screenTexture, TexCoords).rgb;

    vec3 mapped = aces_tonemap(hdr_color);

    FragColor = vec4(mapped, 1.0);
}