#version 460 core
out vec4 FragColor;

in vec3 norm;

uniform vec3 lightColor;

uniform vec3 direction;

void main() {

	float weight = direction == vec3(0.0) ? 1.0 : max(dot(normalize(direction), norm), 0.0);
	vec3 result = lightColor * weight;

	FragColor = vec4(result, 1.0);
}