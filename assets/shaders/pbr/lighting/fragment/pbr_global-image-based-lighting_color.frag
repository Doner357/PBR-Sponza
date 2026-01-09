#version 410 core

in VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
	mat3 TBN;
	mat3 inverse_TBN;
} fs_in;


out vec4 FragColor;

// Properties struct
//------------------------------
// --material--
// PBR factors
struct Material {
	vec3  albedo;
	float metallic;
	float roughness;
	float ao;
};

// --direction light--
struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
#define NUM_OF_DIRLIGHTS 4

// --point light--
struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
#define NUM_OF_POINTLIGHTS 32

// --spot light
struct SpotLight {
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

// --environment light--
struct EnvLight {
	samplerCube irradiance;
	samplerCube prefiltered;
	sampler2D   preBrdf;
};

#define NO_LIGHT vec3(0.0)
#define PI 3.14159265359


// ** Function for Cook-Torrance BRDF **
// Fresnel-Schlick function for F
vec3 FresnelSchlick(float cosTheta, vec3 F0);
// Fresnel-Schlick function, but take roughness into account
vec3 FresnelSchlick(float cosTheta, vec3 F0, float roughness);
// Trowbridge-Reitz GG normal distribution function for D
float DistributionGGX(vec3 N, vec3 H, float roughness);
// Schlick-GGX geometry function
float GeometrySchlickGGX(float N_dot_V, float roughness);
// GeometrySmith geometry function for G
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);


// Function calculate each type of light
vec3 CalcDirLight(DirLight light, vec3 N, vec3 V, vec3 F0);
vec3 CalcPointLight(PointLight light, vec3 fragPos, vec3 N, vec3 V, vec3 F0);
vec3 CalcSpotLight(SpotLight light, vec3 fragPos, vec3 N, vec3 V, vec3 F0);
vec3 CalcEnvLight(vec3 N, vec3 V, vec3 F0);


uniform Material material;
uniform vec3 viewPos;
layout (std140) uniform GlobalLights {               // size      ali
	DirLight dirLights[NUM_OF_DIRLIGHTS];            //  256        0
	PointLight pointLights[NUM_OF_POINTLIGHTS];      // 2560	  256
	SpotLight spotLights[NUM_OF_SPOTLIGHTS];         //  768     2816
}; // total 3584

// Environment map
uniform EnvLight environment;


void main() {
	// Result
	vec3 Lo = vec3(0.0);

	// Properties
	// Normal
	vec3 N  = normalize(fs_in.normal);
	// View direction
	vec3 V  = normalize(viewPos - fs_in.fragPos);
	// Surface reflection at zero incidence
	vec3 F0 = vec3(0.04);
	F0      = mix(F0, material.albedo, material.metallic);

	// Phase 1: Directional lighting
	for(int i = 0; i < NUM_OF_DIRLIGHTS; i++)
		Lo += dirLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcDirLight(dirLights[i], N, V, F0);
	// Phase 2: Point lights
	for(int i = 0; i < NUM_OF_POINTLIGHTS; i++)
		Lo += pointLights[i].position == NO_LIGHT ? vec3(0.0) : CalcPointLight(pointLights[i], fs_in.fragPos, N, V, F0);
	// Phase 3: Spot light
	for(int i = 0; i < NUM_OF_SPOTLIGHTS; i++)
		Lo += spotLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcSpotLight(spotLights[i], fs_in.fragPos, N, V, F0);
	
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

vec3 CalcDirLight(DirLight light, vec3 N, vec3 V, vec3 F0) {
	// Result for outgoing radiance
	vec3 Lo = vec3(0.0);

	// Light direction
	vec3 L = normalize(-light.direction);
	// Half vector
	vec3 H = normalize(V + L);

	// In fact light.specular is light color
	vec3  radians = light.specular;

	// Fresnel-Schlick
	vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);
	float NDF = DistributionGGX(N, H, material.roughness);
	float G   = GeometrySmith(N, V, L, material.roughness);

	// Calculate the ratio for diffusion, F is for specular contribution
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	// Because metallic surfaces don't refract light and thus have no diffuse
	// reflections we enforce this property by nullifying kD if the surface is metallic.
	kD *= 1.0 - material.metallic;

	// Compute Cook-Torrance BRDF
	vec3  numerator   = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.000001;
	vec3  specular    = numerator / denominator;

	// Calculate diffuse term and result
	float N_dot_L = max(dot(N, L), 0.0);
	Lo += (kD * material.albedo / PI + specular) * radians * N_dot_L;

	// Calculate ambient light
	vec3 ambient = light.ambient * material.albedo * material.ao;

	return ambient + Lo;
}

vec3 CalcPointLight(PointLight light, vec3 fragPos, vec3 N, vec3 V, vec3 F0) {
	// Result for outgoing radiance
	vec3 Lo = vec3(0.0);

	// Light direction
	vec3 L = normalize(light.position - fs_in.fragPos);
	// Half vector
	vec3 H = normalize(V + L);

	// Do light attenuation
	float distance    = length(light.position - fs_in.fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	// In fact light.specular is light color
	vec3  radians     = light.specular * attenuation;

	// Fresnel-Schlick
	vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);
	float NDF = DistributionGGX(N, H, material.roughness);
	float G   = GeometrySmith(N, V, L, material.roughness);

	// Calculate the ratio for diffusion, F is for specular contribution
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	// Because metallic surfaces don't refract light and thus have no diffuse
	// reflections we enforce this property by nullifying kD if the surface is metallic.
	kD *= 1.0 - material.metallic;

	// Compute Cook-Torrance BRDF
	vec3  numerator   = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0000001;
	vec3  specular    = numerator / denominator;

	// Calculate diffuse term and result
	float N_dot_L = max(dot(N, L), 0.0);
	Lo += (kD * material.albedo / PI + specular) * radians * N_dot_L;

	// Calculate ambient light
	vec3 ambient = light.ambient * material.albedo * material.ao;
	ambient *= attenuation;

	return ambient + Lo;
}

vec3 CalcSpotLight(SpotLight light, vec3 fragPos, vec3 N, vec3 V, vec3 F0) {
	// Result for outgoing radiance
	vec3 Lo = vec3(0.0);

	// Light direction
	vec3 L = normalize(light.position - fs_in.fragPos);
	// Half vector
	vec3 H = normalize(V + L);

	// Do light attenuation
	float distance    = length(light.position - fs_in.fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	// --spotlight intensity--
	float theta     = dot(L, normalize(-light.direction));   // the cosine of the angle between spotlight direction and light direction
	float epsilon   = light.innerCutOff - light.outerCutOff;      // the cosine difference between the inner cone and outer cone
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);    // the intensity of spotlight
	
	// In fact light.specular is light color
	vec3  radians     = light.specular * attenuation * intensity;

	// Fresnel-Schlick
	vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);
	float NDF = DistributionGGX(N, H, material.roughness);
	float G   = GeometrySmith(N, V, L, material.roughness);

	// Calculate the ratio for diffusion, F is for specular contribution
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	// Because metallic surfaces don't refract light and thus have no diffuse
	// reflections we enforce this property by nullifying kD if the surface is metallic.
	kD *= 1.0 - material.metallic;

	// Compute Cook-Torrance BRDF
	vec3  numerator   = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.000001;
	vec3  specular    = numerator / denominator;

	// Calculate diffuse term and result
	float N_dot_L = max(dot(N, L), 0.0);
	Lo += (kD * material.albedo / PI + specular) * radians * N_dot_L;

	// Calculate ambient light
	vec3 ambient = light.ambient * material.albedo * material.ao;
	ambient *= attenuation;

	return ambient + Lo;
}

vec3 CalcEnvLight(vec3 N, vec3 V, vec3 F0) {
	// Fresnel term
	vec3 kS = FresnelSchlick(max(dot(N, V), 0.0), F0, material.roughness);
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - material.metallic;

	// 
	// ** Diffuse term **
	// 
	// Get irradiance from irradiance map
	vec3 irradiance = texture(environment.irradiance, N).rgb;
	vec3 diffuse    = irradiance * material.albedo;

	
	// 
	// ** Specular term **
	// 
	vec3 R = normalize(reflect(-V, N));
	// Sample the pre-filtered environment map according to the reflection vector and material roughness
	const float kMaxReflectionLod = 4.0;
	vec3 prefiltered_color = textureLod(environment.prefiltered, R, material.roughness * kMaxReflectionLod).rgb;
	
	// Sample the LUT according to the normal and view vector
	vec2 env_brdf = texture(environment.preBrdf, vec2(max(dot(N, V), 0.0), material.roughness)).rg;
	vec3 specular = prefiltered_color * (kS * env_brdf.x + env_brdf.y);


	// Combind both term to final result
	vec3 ambient = (kD * diffuse + specular) * material.ao;

	return ambient;
}