#version 410 core

in VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
	vec4 dirLightSpacePos[1];
	vec4 spotLightSpacePos[2];
} fs_in;

out vec4 FragColor;


// Properties struct
//------------------------------
// --material--
struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	float shininess;
};


// --direction light--
struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
#define NUM_OF_DIRLIGHTS 4
#define NUM_OF_SHADOWDIRLIGHTS 1


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
#define NUM_OF_SHADOWPOINTLIGHTS 4


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
#define NUM_OF_SHADOWSPOTLIGHTS 2


// Used to bind depth map
struct ShadowMaps {
	sampler2D dirLights[NUM_OF_SHADOWDIRLIGHTS];
	samplerCube pointLights[NUM_OF_SHADOWPOINTLIGHTS];
	sampler2D spotLights[NUM_OF_SHADOWSPOTLIGHTS];
};


#define NO_LIGHT vec3(0.0)


// Shadow calculation function
//------------------------------
float DirLightShadowCalculation(DirLight light, vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int shadow_id);
float PointLightShadowCalculation(PointLight light, vec3 fragPos, vec3 normal, int shadow_id);
float SpotLightShadowCalculation(SpotLight light, vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int shadow_id);

// Lighting calculation function
//------------------------------
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcShadowDirLight(DirLight light, vec3 normal, vec3 viewDir, int shadow_id);
vec3 CalcShadowPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, int shadow_id);
vec3 CalcShadowSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, int shadow_id);


// Uniform
//------------------------------
uniform Material material;
uniform vec3 viewPos;

layout (std140) uniform GlobalLights {               // size      ali
	DirLight dirLights[NUM_OF_DIRLIGHTS];            //  256        0
	PointLight pointLights[NUM_OF_POINTLIGHTS];      // 2560	  256
	SpotLight spotLights[NUM_OF_SPOTLIGHTS];         //  768     2816
}; // total 3584

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
uniform ShadowMaps shadowMaps;


void main() {
	// Properties
	vec3 normal = normalize(fs_in.normal);
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	vec3 result = vec3(0.0);

	// Light wiht no shadow
	for(int i = 0; i < NUM_OF_DIRLIGHTS; i++)
		result += dirLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcDirLight(dirLights[i], normal, viewDir);
	for(int i = 0; i < NUM_OF_POINTLIGHTS; i++)
		result += pointLights[i].position == NO_LIGHT ? vec3(0.0) : CalcPointLight(pointLights[i], normal, fs_in.fragPos, viewDir);
	for(int i = 0; i < NUM_OF_SPOTLIGHTS; i++)
		result += spotLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcSpotLight(spotLights[i], normal, fs_in.fragPos, viewDir);

	// Light with shadow
	for (int i = 0; i < NUM_OF_SHADOWDIRLIGHTS; i++)
		result += shadowDirLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcShadowDirLight(shadowDirLights[i], normal, viewDir, i);
	for (int i = 0; i < NUM_OF_SHADOWPOINTLIGHTS; i++)
		result += shadowPointLights[i].position == NO_LIGHT ? vec3(0.0) : CalcShadowPointLight(shadowPointLights[i], normal, fs_in.fragPos, viewDir, i);
	for (int i = 0; i < NUM_OF_SHADOWSPOTLIGHTS; i++)
		result += shadowSpotLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcShadowSpotLight(shadowSpotLights[i], normal, fs_in.fragPos, viewDir, i);

	float alpha = texture(material.texture_diffuse1, fs_in.texCoords).a;
	FragColor = vec4(result, alpha);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {

	// Basic data calculation
	//--------------------------
	vec3 lightDir = normalize(-light.direction);   // light's direction
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	// Lighting Calculation
	//--------------------------
	// --ambient--
	vec3 ambient = light.ambient * texture(material.texture_diffuse1, fs_in.texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, fs_in.texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.texture_specular1, fs_in.texCoords).rgb;

	// Result
	//--------------------------
	return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	// Basic data calculation
	//--------------------------
	vec3 lightDir = normalize(light.position - fragPos);   // light direction
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	// Lighting Calculation
	//--------------------------
	// --ambient--
	vec3 ambient = light.ambient * texture(material.texture_diffuse1, fs_in.texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, fs_in.texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.texture_specular1, fs_in.texCoords).rgb;

	// --attenuation--
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	// Result
	//--------------------------
	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;
	return ambient + diffuse + specular;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	// Basic data calculation
	//--------------------------
	vec3 lightDir = normalize(light.position - fragPos);        // light direction
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	
	// Lighting Calculation
	//--------------------------
	// --ambient--
	vec3 ambient = light.ambient * texture(material.texture_diffuse1, fs_in.texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, fs_in.texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.texture_specular1, fs_in.texCoords).rgb;

	// --attenuation--
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	// --spotlight intensity--
	float theta     = dot(lightDir, normalize(-light.direction));                // the cosine of the angle between spotlight direction and light direction
	float epsilon   = light.innerCutOff - light.outerCutOff;                     // the cosine difference between the inner cone and outer cone
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);    // the intensity of spotlight


	// Result
	//--------------------------
	ambient  *= attenuation;
	diffuse  *= attenuation * intensity;
	specular *= attenuation * intensity;

	return ambient + diffuse + specular;
}

vec3 CalcShadowDirLight(DirLight light, vec3 normal, vec3 viewDir, int shadow_id) {

	// Basic data calculation
	//--------------------------
	vec3 lightDir = normalize(-light.direction);   // light's direction
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	// Lighting Calculation
	//--------------------------
	// --ambient--
	vec3 ambient = light.ambient * texture(material.texture_diffuse1, fs_in.texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, fs_in.texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.texture_specular1, fs_in.texCoords).rgb;

	// --shadow--
	float shadow = DirLightShadowCalculation(light, fs_in.dirLightSpacePos[shadow_id], normal, lightDir, shadow_id);

	// Result
	//--------------------------
	return ambient + (1 - shadow) * (diffuse + specular);	
}

vec3 CalcShadowPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, int shadow_id) {

	// Basic data calculation
	//--------------------------
	vec3 lightDir = normalize(light.position - fragPos);   // light's direction
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	// Lighting Calculation
	//--------------------------
	// --ambient--
	vec3 ambient = light.ambient * texture(material.texture_diffuse1, fs_in.texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, fs_in.texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.texture_specular1, fs_in.texCoords).rgb;
	
	// --attenuation--
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	// Result
	//--------------------------
	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;

	// --shadow--
	float shadow = PointLightShadowCalculation(light, fragPos, normal, shadow_id);

	// Result
	//--------------------------
	return ambient + (1.0 - shadow) * (diffuse + specular);	
}

vec3 CalcShadowSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, int shadow_id) {
	// Basic data calculation
	//--------------------------
	vec3 lightDir = normalize(light.position - fragPos);        // light direction
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	
	// Lighting Calculation
	//--------------------------
	// --ambient--
	vec3 ambient = light.ambient * texture(material.texture_diffuse1, fs_in.texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, fs_in.texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.texture_specular1, fs_in.texCoords).rgb;

	// --attenuation--
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	// --spotlight intensity--
	float theta     = dot(lightDir, normalize(-light.direction));                // the cosine of the angle between spotlight direction and light direction
	float epsilon   = light.innerCutOff - light.outerCutOff;                     // the cosine difference between the inner cone and outer cone
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);    // the intensity of spotlight

	// Result
	//--------------------------
	ambient  *= attenuation;
	diffuse  *= attenuation * intensity;
	specular *= attenuation * intensity;

	// --shadow--
	float shadow = SpotLightShadowCalculation(light, fs_in.spotLightSpacePos[shadow_id], normal, lightDir, shadow_id);

	return ambient + (1.0 - shadow) * (diffuse + specular);
}

float DirLightShadowCalculation(DirLight light, vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int shadow_id) {
	// Perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Transform NDC coordinates to the range [0, 1]
	projCoords = projCoords * 0.5 + 0.5;
	// Get current fragment's depth from light view space
	float currentDepth = projCoords.z;

	// Calculate depth bias according to the angle between light direction and surface normal
	float bias = 0.02 - 0.015 * dot(normal, lightDir);
	
	// Apply PCF (percentage-closer filtering)
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMaps.dirLights[shadow_id], 0);    // Used to offset the sample
	for (int x = -1; x <= 1; x++) {
		for(int y = -1; y <= 1; y++) {
			// Get the closest point from light view space
			float pcfDepth = texture(shadowMaps.dirLights[shadow_id], projCoords.xy + vec2(x, y) * texelSize).r;
			// Check whether the current depth is higher than closest depth and if so, it's in ShadowDirLight
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	// Normalize the shadow value
	shadow /= 9.0;
	
	// Avoid the sample fragment is exceed the project far plane. If it is exceed, assume it is not in shadow
	if (projCoords.z > 1.0)
		shadow = 0.0;

	return shadow;
}

float PointLightShadowCalculation(PointLight light, vec3 fragPos, vec3 normal, int shadow_id) {
	// Get the vector from light to fragment position
	vec3 fragToLight = fragPos - light.position;
	// Get the depth value from current fragment to light source
	float currentDepth = length(fragToLight);

	// It is too costly to sample 4 * 4 * 4 = 64 samples, instead we create an offset array to cheap the sampling
	float shadow  = 0.0;
	float bias    = 0.02 - 0.015 * dot(normal, normalize(fragToLight));    // 0.005 + (0.015 * (1 - dot(normal, normalize(fragToLight))))
	int samples   = 20;
	float viewDistance = length(viewPos - fragPos);
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
		// Get the closest depth perspect from light
		float closestDepth = texture(shadowMaps.pointLights[shadow_id], fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		// Trans the light's depth from [0, 1] back to [0, far_plane]
		closestDepth *= shadowPointLight_far_planes[shadow_id];
		if (currentDepth - bias > closestDepth)
			shadow += 1.0;
	}
	shadow /= float(samples);

	return shadow;
}

float SpotLightShadowCalculation(SpotLight light, vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int shadow_id) {
	// Perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Transform NDC coordinates to the range [0, 1]
	projCoords = projCoords * 0.5 + 0.5;
	// Get current fragment's depth from light view space
	float currentDepth = projCoords.z;

	// Calculate depth bias according to the angle between light direction and surface normal
	float bias = 0.0002 - 0.00015 * dot(normal, lightDir);
	
	// Apply PCF (percentage-closer filtering)
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMaps.spotLights[shadow_id], 0);    // Used to offset the sample
	for (int x = -1; x <= 1; x++) {
		for(int y = -1; y <= 1; y++) {
			// Get the closest point from light view space
			float pcfDepth = texture(shadowMaps.spotLights[shadow_id], projCoords.xy + vec2(x, y) * texelSize).r;
			// Check whether the current depth is higher than closest depth and if so, it's in ShadowDirLight
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	// Normalize the shadow value
	shadow /= 9.0;
	
	// Avoid the sample fragment is exceed the project far plane. If it is exceed, assume it is not in shadow
	if (projCoords.z > 1.0)
		shadow = 0.0;

	return shadow;
}