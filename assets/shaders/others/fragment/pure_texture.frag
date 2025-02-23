#version 460 core

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D texture1;

// Used for gamma correction
layout (std140) uniform GammaCorrection {
	float gamma;    // 4 bytes
};

void main() {
	vec3 texColor = texture(texture1, TexCoords).rgb;
	
	// Gamma correction
	float gam = gamma == 0.0 ? 1.0 : gamma;    // Avoid 0 as exponent
	texColor = pow(texColor, vec3(1.0 / gam));

	float alpha = texture(texture1, TexCoords).a;
	FragColor = vec4(texColor, alpha);
}