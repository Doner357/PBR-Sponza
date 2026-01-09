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
	sampler2D  albedo;
	sampler2D  metallic;
	sampler2D  roughness;
	sampler2D  normal;
	sampler2D  ao;
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
vec3 CalcDirLight(DirLight light, vec3 N, vec3 V, vec3 F0);
vec3 CalcPointLight(PointLight light, vec3 N, vec3 V, vec3 F0);
vec3 CalcSpotLight(SpotLight light, vec3 N, vec3 V, vec3 F0);


uniform Material material;
uniform vec3 viewPos;
layout (std140) uniform GlobalLights {               // size      ali
	DirLight dirLights[NUM_OF_DIRLIGHTS];            //  256        0
	PointLight pointLights[NUM_OF_POINTLIGHTS];      // 2560	  256
	SpotLight spotLights[NUM_OF_SPOTLIGHTS];         //  768     2816
}; // total 3584


void main() {
	// Result
	vec3 Lo = vec3(0.0);

	// Properties
	// View direction
	vec3 V  = normalize(viewPos - fs_in.fragPos);
	// Normal
	vec3 N  = GetNormalFromMap();
	// Surface reflection at zero incidence
	vec3  albedo   = texture(material.albedo, fs_in.texCoords).rgb;
	float metallic = texture(material.metallic, fs_in.texCoords).r;
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	// Phase 1: Directional lighting
	for(int i = 0; i < NUM_OF_DIRLIGHTS; i++)
		Lo += dirLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcDirLight(dirLights[i], N, V, F0);
	// Phase 2: Point lights
	for(int i = 0; i < NUM_OF_POINTLIGHTS; i++)
		Lo += pointLights[i].position == NO_LIGHT ? vec3(0.0) : CalcPointLight(pointLights[i], N, V, F0);
	// Phase 3: Spot light
	for(int i = 0; i < NUM_OF_SPOTLIGHTS; i++)
		Lo += spotLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcSpotLight(spotLights[i], N, V, F0);
	
	FragColor = vec4(Lo, 1.0);
}

vec3 GetNormalFromMap() {
    vec3 tangentNormal = texture(material.normal, fs_in.texCoords).rgb * 2.0 - 1.0;
    return normalize(fs_in.TBN * tangentNormal);
}


vec3 FresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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
	// Extract material attribution from texture
	vec3  albedo    = texture(material.albedo, fs_in.texCoords).rgb;
	float metallic  = texture(material.metallic, fs_in.texCoords).r;
	float roughness = texture(material.roughness, fs_in.texCoords).r;
	float ao        = texture(material.ao, fs_in.texCoords).r;

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

	return ambient + Lo;
}

vec3 CalcPointLight(PointLight light, vec3 N, vec3 V, vec3 F0) {
	// Extract material attribution from texture
	vec3  albedo    = texture(material.albedo, fs_in.texCoords).rgb;
	float metallic  = texture(material.metallic, fs_in.texCoords).r;
	float roughness = texture(material.roughness, fs_in.texCoords).r;
	float ao        = texture(material.ao, fs_in.texCoords).r;

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

	return ambient + Lo;
}

vec3 CalcSpotLight(SpotLight light, vec3 N, vec3 V, vec3 F0) {
	// Extract material attribution from texture
	vec3  albedo    = texture(material.albedo, fs_in.texCoords).rgb;
	float metallic  = texture(material.metallic, fs_in.texCoords).r;
	float roughness = texture(material.roughness, fs_in.texCoords).r;
	float ao        = texture(material.ao, fs_in.texCoords).r;

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

	return ambient + Lo;
}