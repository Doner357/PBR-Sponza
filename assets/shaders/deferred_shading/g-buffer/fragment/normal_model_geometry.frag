#version 410 core

in VS_OUT {
    vec3 fragPos;
    vec2 texCoords;
    mat3 TBN;
	mat3 inverse_TBN;
} fs_in;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormalShininess;
layout (location = 2) out vec4 gAlbedoSpec;

// Properties struct
//------------------------------
struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_normal1;
	sampler2D texture_height1;
	float shininess;
};

uniform Material material;


void main() {
	// Store the fragment position vector in the first gbuffer texture
	gPosition = fs_in.fragPos;
	// also store the per-fragment normals into the gbuffer
	vec3 normal = texture(material.texture_normal1, fs_in.texCoords).rgb;
	normal = normal * 2.0 - 1.0;
	gNormalShininess.rgb = normalize(fs_in.TBN * normal);
	// Put shininess into alpha channel of normal buffer
	gNormalShininess.a   = material.shininess;
	// and the diffuse per-fragment color
	gAlbedoSpec.rgb = texture(material.texture_diffuse1, fs_in.texCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
	gAlbedoSpec.a = texture(material.texture_specular1, fs_in.texCoords).r;
}