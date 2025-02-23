#version 330 core

layout (location = 0) out vec4 FragColor;

in VS_OUT {
	vec2 texCoords;
} fs_in;

uniform sampler2D baseColor;
uniform sampler2D ssrColor;
uniform sampler2D aoTexture;

uniform float blendStrength;

void main() {
    vec3 base = texture(baseColor, fs_in.texCoords).rgb;
    vec3 ssr  = texture(ssrColor, fs_in.texCoords).rgb;
	float ao = texture(aoTexture, fs_in.texCoords).r;

    vec3 result = base + ssr * ao * blendStrength;
    FragColor = vec4(result, 1.0);
}