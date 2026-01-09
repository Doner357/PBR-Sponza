#version 410 core

layout (location = 0) out vec3 gAlbedo;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gMetarial;
layout (location = 3) out vec3 gPosition;

in VS_OUT {
	vec3 fragPos;
	vec3 fragPosW;
	vec3 normal;
	vec3 normalW;
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
// --direction light--
struct DirLight {
	bool activate;

	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
#define NUM_OF_DIRLIGHTS 4
#define NUM_OF_SHADOWDIRLIGHTS 4
#define RAIN_FLOOR_INDEX NUM_OF_SHADOWDIRLIGHTS - 1

// --point light--
struct PointLight {
	bool activate;

	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
#define NUM_OF_POINTLIGHTS 32
#define NUM_OF_SHADOWPOINTLIGHTS 16

struct SpotLight {
	bool activate;

	vec3 position;
	vec3 direction;

	float innerCutOff;
	float outerCutOff;
	
	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
#define NUM_OF_SPOTLIGHTS 8
#define NUM_OF_SHADOWSPOTLIGHTS 16

// Used to bind depth map
layout (std140) uniform ShadowMatrices {
	mat4 dirLightSpaceMat[NUM_OF_SHADOWDIRLIGHTS];
	mat4 spotLightSpaceMat[NUM_OF_SHADOWSPOTLIGHTS];
};
// Record the point light's far plane
layout (std140) uniform ShadowFarPlanes {                           // Each element in array has a base alignment equal to that of a vec4.
	float shadowPointLight_far_planes[NUM_OF_SHADOWPOINTLIGHTS];    //   16 * 4 = 64        0
};
// Record the lights need to calculate shadow
layout (std140) uniform GlobalShadowLights {                        // size      ali
	DirLight shadowDirLights[NUM_OF_SHADOWDIRLIGHTS];               //   64        0
	PointLight shadowPointLights[NUM_OF_SHADOWPOINTLIGHTS];         //  320       64
	SpotLight shadowSpotLights[NUM_OF_SHADOWSPOTLIGHTS];            //  192      384
}; // total 576
struct ShadowMaps {
	sampler2DArrayShadow dirLights;
	samplerCubeArrayShadow pointLights;
	sampler2DArrayShadow spotLights;
};

uniform Material material;
uniform ShadowMaps shadowMaps;
uniform bool rainFloor;

float wetFloorCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int shadow_id) {
	// Perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Transform NDC coordinates to the range [0, 1]
	projCoords = projCoords * 0.5 + 0.5;
	// Get current fragment's depth from light view space
	float currentDepth = projCoords.z;

	// Calculate depth bias according to the angle between light direction and surface normal
	float bias = 0.01;
	
	// Apply PCF (percentage-closer filtering)
	float shadow = 0.0;
	vec2 mapSize = textureSize(shadowMaps.dirLights, 0).st;
	vec2 texelSize = 1.0 / mapSize * 2.0;    // Used to offset the sample
	for (float x = -6.0; x <= 6.0; x += 2.0) {
		for(float y = -6.0; y <= 6.0; y += 2.0) {
			shadow += texture(shadowMaps.dirLights, vec4(projCoords.xy + vec2(x, y) * texelSize, float(shadow_id), currentDepth - bias));
		}
	}
	
	// Normalize the shadow value
	shadow *= 0.0625;
	
	// Avoid the sample fragment is exceed the project far plane. If it is exceed, assume it is not in shadow
	if (projCoords.z > 1.0)
		shadow = 0.0;

	return shadow * 0.3;
}


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
    // store material info
	vec4 light_space_frag = dirLightSpaceMat[RAIN_FLOOR_INDEX] * vec4(fs_in.fragPosW, 1.0);
	vec3 normal_w = normalize(fs_in.normalW);
	vec3 light_dir = normalize(shadowDirLights[RAIN_FLOOR_INDEX].direction);
	// Check rainy
	normal = normalize(fs_in.TBN * normal);
	if (rainFloor) {
		float in_rain_weight = wetFloorCalculation(light_space_frag, normal_w, light_dir, RAIN_FLOOR_INDEX);
		roughness -= in_rain_weight;
		roughness = max(roughness, 0.0);
		if (normal_w.y >= 0.8) {
			normal = normalize(mix(normal, fs_in.normal, in_rain_weight));
		}
	}
	gNormal = normal;
	gMetarial = vec3(metallic, roughness, ao);
	// Store the fragment position vector in the first gbuffer texture
	gPosition = fs_in.fragPos;
}