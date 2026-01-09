#version 410 core
in vec4 FragPos;

in vec2 gtexCoords;

struct Material {
	sampler2D  albedo;
	sampler2D  metallic;
	sampler2D  roughness;
	sampler2D  normal;
	sampler2D  ao;
	sampler2D  opacity;
};

uniform Material material;

uniform vec3 lightPos;
uniform float far_plane;

void main() {
	float alpha = texture(material.albedo, gtexCoords).a;
	float opacity = texture(material.opacity, gtexCoords).r;
	if (alpha <= 0.0001 || opacity <= 0.0001) {
		discard;		
	}

	// Get distance between fragment and light source
	float lightDistance = length(FragPos.xyz - lightPos);

	// Map to [0;1] range by dividing by far_planel
	lightDistance = lightDistance / far_plane;

	// Write this as modified depth
	gl_FragDepth = lightDistance;
}