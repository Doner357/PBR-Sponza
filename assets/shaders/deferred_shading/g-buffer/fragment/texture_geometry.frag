#version 410 core

in VS_OUT {
    vec3 fragPos;
	vec3 normal;
    vec2 texCoords;
} fs_in;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormalShininess;
layout (location = 2) out vec4 gAlbedoSpec;

// Properties struct
//------------------------------
struct Material {
	sampler2D diffuse1;
	sampler2D specular1;
	float shininess;
};

uniform Material material;


void main() {
	// Store the fragment position vector in the first gbuffer texture
	gPosition = fs_in.fragPos;
	// also store the per-fragment normals into the gbuffer
	gNormalShininess.rgb = normalize(fs_in.normal);
	// Put shininess into alpha channel of normal buffer
	gNormalShininess.a   = material.shininess;
	// and the diffuse per-fragment color
	gAlbedoSpec.rgb = texture(material.diffuse1, fs_in.texCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
	gAlbedoSpec.a = texture(material.specular1, fs_in.texCoords).r;
}