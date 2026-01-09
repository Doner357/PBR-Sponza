#version 410 core

in vec2 TexCoords;

out vec4 FragColor;

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	float shininess;
};

uniform Material material;

// Used for gamma correction
layout (std140) uniform GammaCorrection {
	float gamma;    // 4 bytes
};

void main() {
	vec3 result = texture(material.texture_diffuse1, TexCoords).rgb;
	
	// Gamma correction
	float gam = gamma == 0.0 ? 1.0 : gamma;    // Avoid the 0 exponent
	result = pow(result, vec3(1.0 / gam));

	float alpha = texture(material.texture_diffuse1, TexCoords).a;
	FragColor = vec4(result, alpha);
}