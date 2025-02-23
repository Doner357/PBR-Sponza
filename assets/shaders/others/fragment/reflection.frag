#version 460 core
in vec3 Position;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 viewPos;
uniform samplerCube cubemap;

// Used for gamma correction
layout (std140) uniform GammaCorrection {
	float gamma;    // 4 bytes
};

void main() {
	vec3 viewDir = normalize(Position - viewPos);
	vec3 reflectDir = reflect(viewDir, normalize(Normal));

	vec3 result = texture(cubemap, reflectDir).rgb;

	// Gamma correction
	float gam = gamma == 0.0 ? 1.0 : gamma;    // Avoid 0 as exponent
	result = pow(result, vec3(1.0 / gam));
	
	float alpha = texture(cubemap, reflectDir).a;
	FragColor = vec4(result, alpha);
}