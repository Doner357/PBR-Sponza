#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
	vec4 color = texture(screenTexture, TexCoords);

	FragColor = vec4(color.rgb, 1.0);    // Temporarily changed to output ssao texture
}