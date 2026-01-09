#version 410 core
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D screenTexture;

// Used for gamma correction
layout (std140) uniform GammaCorrection {
	float gamma;    // 4 bytes
};

void main() {
	vec3 result = texture(screenTexture, TexCoords).rgb;

	float gam = (gamma == 0.0 ? 1.0 : gamma);    // Avoid 0 exponent
	result = pow(result, vec3(1.0 / gam));

	float alpha = texture(screenTexture, TexCoords).a;
	FragColor = vec4(result, alpha);
}