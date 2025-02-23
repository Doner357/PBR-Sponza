#version 460 core

in VS_OUT {
	vec2 texCoords;
} fs_in;

out vec4 FragColor;

// Properties struct
//------------------------------
// --material--
// PBR factors
struct GBuffer {
	sampler2D  albedo;
	sampler2D  normal;
	sampler2D  material;
	sampler2D  position;
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
#define NUM_OF_SHADOWDIRLIGHTS 4 // Minus one for rain shadow

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

// --environment light--
struct EnvLight {
	samplerCube irradiance;
	samplerCube prefiltered;
	sampler2D   preBrdf;
};

#define NO_LIGHT vec3(0.0)
#define PI 3.14159265359


// Get normal from normal map
vec3 GetNormalFromMap();

// ** Function for Cook-Torrance BRDF **
// Fresnel-Schlick function for F
vec3 FresnelSchlick(float cosTheta, vec3 F0);
// Trowbridge-Reitz GG normal distribution function for D
float DistributionGGX(vec3 N, vec3 H, float roughness);
// Schlick-GGX geometry function
float GeometrySchlickGGX(float N_dot_V, float roughness);
// GeometrySmith geometry function for G
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);

// Function calculate each type of light
vec3 CalcEnvLight(vec3 N, vec3 V, vec3 F0);

vec3 CalcShadowDirLight(DirLight light, vec3 N, vec3 V, vec3 F0, int shadow_id);
vec3 CalcShadowPointLight(PointLight light, vec3 N, vec3 V, vec3 F0, int shadow_id);
vec3 CalcShadowSpotLight(SpotLight light, vec3 N, vec3 V, vec3 F0, int shadow_id);

float DirLightShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int shadow_id);
float PointLightShadowCalculation(vec3 fragPos, vec3 normal, vec3 lightPos, int shadow_id);
float SpotLightShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int shadow_id);


layout (std140) uniform CameraMatrices {
	mat4 view;
	mat4 projection;
};
uniform mat3 vecWorldToView;
uniform mat3 vecViewToWorld;
uniform mat4 viewToWorld;
uniform GBuffer gbuffer;
uniform sampler2D ssao_texture;

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
uniform ShadowMaps shadowMaps;


// Environment map
uniform EnvLight environment;


void main() {
	vec3 albedo = texture(gbuffer.albedo, fs_in.texCoords).rgb;

	// Result
	vec3 Lo = vec3(0.0);

	// Properties
	// View direction
	vec3 fragPos = texture(gbuffer.position, fs_in.texCoords).rgb;
	vec3 V  = normalize(-fragPos);
	// Normal
	vec3 N  = texture(gbuffer.normal, fs_in.texCoords).rgb;
	// Surface reflection at zero incidence
	float metallic = texture(gbuffer.material, fs_in.texCoords).r;
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	// Light with shadow
	for (int i = 0; i < NUM_OF_SHADOWDIRLIGHTS - 1; i++)	// Minus 1 for rain shadow
		Lo += shadowDirLights[i].activate ? CalcShadowDirLight(shadowDirLights[i], N, V, F0, i) : vec3(0.0);
	for (int i = 0; i < NUM_OF_SHADOWPOINTLIGHTS; i++)
		Lo += shadowPointLights[i].activate ? CalcShadowPointLight(shadowPointLights[i], N, V, F0, i) : vec3(0.0);
	for (int i = 0; i < NUM_OF_SHADOWSPOTLIGHTS; i++)
		Lo += shadowSpotLights[i].activate ? CalcShadowSpotLight(shadowSpotLights[i], N, V, F0, i) : vec3(0.0);
	
	N = vecViewToWorld * N;
	V = vecViewToWorld * V;
	Lo += CalcEnvLight(N, V, F0);
	
	FragColor = vec4(Lo, 1.0);
}


vec3 FresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0, float roughness) {
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
	float a        = roughness * roughness;
	float a2       = a * a;
	float N_dot_H  = max(dot(N, H), 0.0);
	float N_dot_H2 = N_dot_H * N_dot_H;

	float num = a2;
	float denom = (N_dot_H2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float N_dot_V, float roughness) {
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num   = N_dot_V;
	float denom = N_dot_V * (1.0 - k) + k;

	return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
	float N_dot_V = max(dot(N, V), 0.0);
	float N_dot_L = max(dot(N, L), 0.0);
	float ggx2    = GeometrySchlickGGX(N_dot_V, roughness);
	float ggx1    = GeometrySchlickGGX(N_dot_L, roughness);

	return ggx1 * ggx2;
}


vec3 CalcEnvLight(vec3 N, vec3 V, vec3 F0) {
	float metallic  = texture(gbuffer.material, fs_in.texCoords).r;
	float roughness = texture(gbuffer.material, fs_in.texCoords).g;
	float ao        = texture(gbuffer.material, fs_in.texCoords).b + texture(ssao_texture, fs_in.texCoords).r;
	vec3  albedo    = texture(gbuffer.albedo, fs_in.texCoords).rgb;
	// Fresnel term
	vec3 kS = FresnelSchlick(max(dot(N, V), 0.0), F0, roughness);
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	// 
	// ** Diffuse term **
	// 
	// Get irradiance from irradiance map
	vec3 irradiance = texture(environment.irradiance, N).rgb;
	vec3 diffuse    = irradiance * albedo;

	
	// 
	// ** Specular term **
	// 
	vec3 R = normalize(reflect(-V, N));
	// Sample the pre-filtered environment map according to the reflection vector and material roughness
	const float kMaxReflectionLod = 4.0;
	vec3 prefiltered_color = textureLod(environment.prefiltered, R, roughness * kMaxReflectionLod).rgb;
	
	// Sample the LUT according to the normal and view vector
	vec2 env_brdf = texture(environment.preBrdf, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 specular = prefiltered_color * (kS * env_brdf.x + env_brdf.y);


	// Combind both term to final result
	vec3 ambient = (kD * diffuse + specular) * ao;

	return ambient;
}


vec3 CalcShadowDirLight(DirLight light, vec3 N, vec3 V, vec3 F0, int shadow_id) {
	// Extract material attribution from texture
	float metallic  = texture(gbuffer.material, fs_in.texCoords).r;
	float roughness = texture(gbuffer.material, fs_in.texCoords).g;
	float ao        = texture(gbuffer.material, fs_in.texCoords).b + texture(ssao_texture, fs_in.texCoords).r;
	vec3  albedo    = texture(gbuffer.albedo, fs_in.texCoords).rgb;
	vec3  fragPos   = texture(gbuffer.position, fs_in.texCoords).rgb;

	roughness = max(roughness, 0.05);

	// Result for outgoing radiance
	vec3 Lo = vec3(0.0);

	// Light direction
	vec3 L = vecWorldToView * normalize(-light.direction);
	// Half vector
	vec3 H = normalize(V + L);

	// In fact light.specular is light color
	vec3  radians = light.specular;

	// Fresnel-Schlick
	vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);
	float NDF = DistributionGGX(N, H, roughness);
	float G   = GeometrySmith(N, V, L, roughness);

	// Calculate the ratio for diffusion, F is for specular contribution
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	// Because metallic surfaces don't refract light and thus have no diffuse
	// reflections we enforce this property by nullifying kD if the surface is metallic.
	kD *= 1.0 - metallic;

	// Compute Cook-Torrance BRDF
	vec3  numerator   = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.000001;
	vec3  specular    = numerator / denominator;

	// Calculate diffuse term and result
	float N_dot_L = max(dot(N, L), 0.0);
	Lo += (kD * albedo / PI + specular) * radians * N_dot_L;

	// Calculate ambient light
	vec3 ambient = light.ambient * albedo * ao;
	
	vec4 light_space_frag_pos = dirLightSpaceMat[shadow_id] * viewToWorld * vec4(fragPos, 1.0);
	float shadow = DirLightShadowCalculation(light_space_frag_pos, N, L, shadow_id);

	return ambient + Lo * shadow;
}


vec3 CalcShadowPointLight(PointLight light, vec3 N, vec3 V, vec3 F0, int shadow_id) {
	// Extract material attribution from texture
	float metallic  = texture(gbuffer.material, fs_in.texCoords).r;
	float roughness = texture(gbuffer.material, fs_in.texCoords).g;
	float ao        = texture(gbuffer.material, fs_in.texCoords).b + texture(ssao_texture, fs_in.texCoords).r;
	vec3  albedo    = texture(gbuffer.albedo, fs_in.texCoords).rgb;
	vec3  fragPos   = texture(gbuffer.position, fs_in.texCoords).rgb;

	roughness = max(roughness, 0.05);

	// Result for outgoing radiance
	vec3 Lo = vec3(0.0);

	// Light direction
	vec3 lightPos = vec3(view * vec4(light.position, 1.0));
	if (lightPos == vec3(0.0)) {
		lightPos + vec3(0.000001);
	}
	vec3 L = normalize(lightPos - fragPos);
	// Half vector
	vec3 H = normalize(V + L);

	// Do light attenuation
	float distance    = length(lightPos - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	// In fact light.specular is light color
	vec3  radians     = light.specular * attenuation;

	// Fresnel-Schlick
	vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);
	float NDF = DistributionGGX(N, H, roughness);
	float G   = GeometrySmith(N, V, L, roughness);

	// Calculate the ratio for diffusion, F is for specular contribution
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	// Because metallic surfaces don't refract light and thus have no diffuse
	// reflections we enforce this property by nullifying kD if the surface is metallic.
	kD *= 1.0 - metallic;

	// Compute Cook-Torrance BRDF
	vec3  numerator   = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0000001;
	vec3  specular    = numerator / denominator;

	// Calculate diffuse term and result
	float N_dot_L = max(dot(N, L), 0.0);
	Lo += (kD * albedo / PI + specular) * radians * N_dot_L;

	// Calculate ambient light
	vec3 ambient = light.ambient * albedo * ao;
	ambient *= attenuation;

	fragPos = vec3(viewToWorld * vec4(fragPos, 1.0));
	N = vecViewToWorld * N;
	float shadow = PointLightShadowCalculation(fragPos, N, light.position, shadow_id);

	return ambient + Lo * shadow;
}

vec3 CalcShadowSpotLight(SpotLight light, vec3 N, vec3 V, vec3 F0, int shadow_id) {
	// Extract material attribution from texture
	float metallic  = texture(gbuffer.material, fs_in.texCoords).r;
	float roughness = texture(gbuffer.material, fs_in.texCoords).g;
	float ao        = texture(gbuffer.material, fs_in.texCoords).b + texture(ssao_texture, fs_in.texCoords).r;
	vec3  albedo    = texture(gbuffer.albedo, fs_in.texCoords).rgb;
	vec3  fragPos   = texture(gbuffer.position, fs_in.texCoords).rgb;

	roughness = max(roughness, 0.05);

	// Result for outgoing radiance
	vec3 Lo = vec3(0.0);

	// Light direction
	vec3 lightPos = vec3(view * vec4(light.position, 1.0));
	vec3 lightDir = vecWorldToView * light.direction;
	if (lightPos == vec3(0.0)) {
		lightPos + vec3(0.000001);
	}
	vec3 L = normalize(lightPos - fragPos);
	// Half vector
	vec3 H = normalize(V + L);

	// Do light attenuation
	float distance    = length(lightPos - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	// --spotlight intensity--
	float theta     = dot(L, normalize(-lightDir));   // the cosine of the angle between spotlight direction and light direction
	float epsilon   = light.innerCutOff - light.outerCutOff;      // the cosine difference between the inner cone and outer cone
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);    // the intensity of spotlight
	
	// In fact light.specular is light color
	vec3  radians     = light.specular * attenuation * intensity;

	// Fresnel-Schlick
	vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);
	float NDF = DistributionGGX(N, H, roughness);
	float G   = GeometrySmith(N, V, L, roughness);

	// Calculate the ratio for diffusion, F is for specular contribution
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	// Because metallic surfaces don't refract light and thus have no diffuse
	// reflections we enforce this property by nullifying kD if the surface is metallic.
	kD *= 1.0 - metallic;

	// Compute Cook-Torrance BRDF
	vec3  numerator   = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.000001;
	vec3  specular    = numerator / denominator;

	// Calculate diffuse term and result
	float N_dot_L = max(dot(N, L), 0.0);
	Lo += (kD * albedo / PI + specular) * radians * N_dot_L;

	// Calculate ambient light
	vec3 ambient = light.ambient * albedo * ao;
	ambient *= attenuation;

	vec4 light_space_frag_pos = spotLightSpaceMat[shadow_id] * viewToWorld * vec4(fragPos, 1.0);
	N = vecViewToWorld * N;
	L = vecViewToWorld * L;
	float shadow = SpotLightShadowCalculation(light_space_frag_pos, N, L, shadow_id);

	return ambient + Lo * shadow;
}


float DirLightShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int shadow_id) {
	// Perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Transform NDC coordinates to the range [0, 1]
	projCoords = projCoords * 0.5 + 0.5;
	// Get current fragment's depth from light view space
	float currentDepth = projCoords.z;

	// Calculate depth bias according to the angle between light direction and surface normal
	float bias = 0.005 - 0.0045 * clamp(dot(normal, lightDir), 0.0, 1.0);
	
	// Apply PCF (percentage-closer filtering)
	float shadow = 0.0;
	vec2 mapSize = textureSize(shadowMaps.dirLights, 0).st;
	vec2 texelSize = 1.0 / mapSize;    // Used to offset the sample
	for (float x = -1.5; x <= 1.5; x += 1.0) {
		for(float y = -1.5; y <= 1.5; y += 1.0) {
			shadow += texture(shadowMaps.dirLights, vec4(projCoords.xy + vec2(x, y) * texelSize, float(shadow_id), currentDepth - bias));
		}
	}
	
	// Normalize the shadow value
	shadow *= 0.0625;
	
	// Avoid the sample fragment is exceed the project far plane. If it is exceed, assume it is not in shadow
	if (projCoords.z > 1.0)
		shadow = 0.0;

	return shadow;
}

float PointLightShadowCalculation(vec3 fragPos, vec3 normal, vec3 lightPos, int shadow_id) {
	// Get the vector from light to fragment position
	vec3 fragToLight = fragPos - lightPos;
	// Get the depth value from current fragment to light source
	float currentDepth = length(fragToLight);

	// It is too costly to sample 4 * 4 * 4 = 64 samples, instead we create an offset array to cheap the sampling
	float shadow  = 0.0;
	float bias    = 0.0005 - 0.000275 * (dot(normal, normalize(fragToLight)) + 1.0) * 0.5;    // 0.005 + (0.015 * (1 - dot(normal, normalize(fragToLight))))
	int   samples   = 20;
	vec3  viewFragPos = texture(gbuffer.position, fs_in.texCoords).rgb;
	float viewDistance = length(-viewFragPos);
	// Change radius based on the distance of the viewer to the fragment,
	// making the shadows softer when far away and sharper when close by.
	float diskRadius = (1.0 + (viewDistance / shadowPointLight_far_planes[shadow_id])) / 25.0;
	vec3 sampleOffsetDirections[20] = vec3[](
		vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
		vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
		vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
		vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
		vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	);
	for (int i = 0; i < samples; i++) {
		shadow += texture(shadowMaps.pointLights, vec4(fragToLight + sampleOffsetDirections[i] * diskRadius, float(shadow_id)), currentDepth / shadowPointLight_far_planes[shadow_id] - bias).r;
	}
	shadow /= float(samples);

	return shadow;
}

float SpotLightShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int shadow_id) {
	// Perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Transform NDC coordinates to the range [0, 1]
	projCoords = projCoords * 0.5 + 0.5;
	// Get current fragment's depth from light view space
	float currentDepth = projCoords.z;

	// Calculate depth bias according to the angle between light direction and surface normal
	float bias = 0.001 - 0.0008 * clamp(dot(normal, lightDir), 0.0, 1.0);
	
	// Apply PCF (percentage-closer filtering)
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMaps.spotLights, 0).st;    // Used to offset the sample
	for (int x = -1; x <= 1; x++) {
		for(int y = -1; y <= 1; y++) {
			shadow += texture(shadowMaps.spotLights, vec4(projCoords.xy + vec2(x, y) * texelSize, shadow_id, currentDepth - bias));
		}
	}
	// Normalize the shadow value
	shadow /= 9.0;
	
	// Avoid the sample fragment is exceed the project far plane. If it is exceed, assume it is not in shadow
	if (projCoords.z > 1.0)
		shadow = 0.0;

	return shadow;
}