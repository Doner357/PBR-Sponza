#version 410 core
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
	float ratio = 1.00 / 1.52;
	vec3 viewDir = normalize(Position - viewPos);
	vec3 refractDir = refract(viewDir, normalize(Normal), ratio);

	vec3 result = texture(cubemap, refractDir).rgb;

	// Gamma correction
	float gam = gamma == 0.0 ? 1.0 : gamma;    // Avoid 0 as exponent
	result = pow(result, vec3(1.0 / gam));
	
	float alpha = texture(cubemap, refractDir).a;
	FragColor = vec4(result, alpha);
}