#version 410 core

in vec2 texCoords;

struct Material {
	sampler2D  albedo;
	sampler2D  metallic;
	sampler2D  roughness;
	sampler2D  normal;
	sampler2D  ao;
	sampler2D  opacity;
};

uniform Material material;

void main() {
	float alpha = texture(material.albedo, texCoords).a;
	float opacity = texture(material.opacity, texCoords).r;
	if (alpha <= 0.0001 || opacity <= 0.0001) {
		discard;		
	}
	// gl_FragDepth = gl_FragCoord.z;
}