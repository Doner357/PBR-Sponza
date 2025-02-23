#version 460 core
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D screenTexture;

void main() {
	float depthValue = texture(screenTexture, TexCoords).r;
	FragColor = vec4(vec3(depthValue), 1.0);
}