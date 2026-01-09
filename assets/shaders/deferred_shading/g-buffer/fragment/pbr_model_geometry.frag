#version 410 core

in VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
	mat3 TBN;
	mat3 inverse_TBN;
} fs_in;

// Properties struct
//------------------------------
// --material--
// PBR factors
struct Material {
	sampler2D  albedo;
	sampler2D  metallic;
	sampler2D  roughness;
	sampler2D  normal;
	sampler2D  ao;
	sampler2D  opacity;
};
uniform Material material;

layout (location = 0) out vec3 gAlbedo;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gMetarial;
layout (location = 3) out vec3 gPosition;


void main() {
	vec4  albedo  = texture(material.albedo, fs_in.texCoords);
	float opacity = texture(material.opacity, fs_in.texCoords).r;
	if (albedo.a <= 0.0001 || opacity <= 0.0001) {
		discard;
	}
    float roughness = texture(material.roughness, fs_in.texCoords).r;
	float metallic  = texture(material.metallic, fs_in.texCoords).r;
	float ao        = texture(material.ao, fs_in.texCoords).r;
	// The albedo per-fragment color
	gAlbedo = albedo.rgb;
	// also store the per-fragment normals into the gbuffer
    vec3 normal = texture(material.normal, fs_in.texCoords).rgb;
	normal = normal * 2.0 - 1.0;
	gNormal = normalize(fs_in.TBN * normal);
    // store material info
	gMetarial = vec3(metallic, roughness, ao);
	// Store the fragment position vector in the first gbuffer texture
	gPosition = fs_in.fragPos;
}