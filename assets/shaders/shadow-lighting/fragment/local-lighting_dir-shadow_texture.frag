#version 410 core

in VS_OUT {
	vec2 texCoords;
	vec3 fragPos;
	vec3 normal;
	vec4 fragPosLightSpace;
} fs_in;


out vec4 FragColor;

// Properties struct
//------------------------------
// --material--
struct Material {
	//vec3 ambient;
	sampler2D diffuse;
	sampler2D specular;
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


// --direction light with shadow--
struct ShadowDirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	sampler2D shadowMap;
};


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


// Shadow calculation function
//------------------------------
float DirLightShadowCalculation(ShadowDirLight light, vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);    // Calculate whether the fragment is in the shadow

// Lighting calculation function
//------------------------------
vec3 CalcShadowDirLight(ShadowDirLight light, vec3 normal, vec3 viewDir);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);                     // Directional light calculation function
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);   // Point light calculation function
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);     // Spot light calculation function


// Uniform
//------------------------------
uniform Material material;                            // Material attributes
uniform vec3 viewPos;                                 // Viewer position
uniform ShadowDirLight dirLight_sh;                   // directional light with shadow
uniform DirLight dirLights[NUM_OF_DIRLIGHTS];         // Directional light
uniform PointLight pointLights[NUM_OF_POINTLIGHTS];   // Point light
uniform SpotLight spotLights[NUM_OF_SPOTLIGHTS];      // Spot light


void main() {
	// Properties
	vec3 normal = normalize(fs_in.normal);
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	vec3 result = vec3(0.0);

	result += dirLight_sh.direction == NO_LIGHT ? vec3(0.0) : CalcShadowDirLight(dirLight_sh, normal, viewDir);

	for(int i = 0; i < NUM_OF_DIRLIGHTS; i++)
		result += dirLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcDirLight(dirLights[i], normal, viewDir);
	for(int i = 0; i < NUM_OF_POINTLIGHTS; i++)
		result += pointLights[i].position == NO_LIGHT ? vec3(0.0) : CalcPointLight(pointLights[i], normal, fs_in.fragPos, viewDir);
	for(int i = 0; i < NUM_OF_SPOTLIGHTS; i++)
		result += spotLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcSpotLight(spotLights[i], normal, fs_in.fragPos, viewDir);

	float alpha = texture(material.diffuse, fs_in.texCoords).a;
	FragColor = vec4(result, alpha);
}

float DirLightShadowCalculation(ShadowDirLight light, vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
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
	vec2 texelSize = 1.0 / textureSize(light.shadowMap, 0);    // Used to offset the sample
	for (int x = -1; x <= 1; x++) {
		for(int y = -1; y <= 1; y++) {
			// Get the closest point from light view space
			float pcfDepth = texture(light.shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
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

vec3 CalcShadowDirLight(ShadowDirLight light, vec3 normal, vec3 viewDir) {

	// Basic data calculation
	//--------------------------
	vec3 lightDir = normalize(-light.direction);   // light's direction
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	// Lighting Calculation
	//--------------------------
	// --ambient--
	vec3 ambient = light.ambient * texture(material.diffuse, fs_in.texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, fs_in.texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.specular, fs_in.texCoords).rgb;

	// --shadow--
	float shadow = DirLightShadowCalculation(light, fs_in.fragPosLightSpace, normal, lightDir);

	// Result
	//--------------------------
	return ambient + (1 - shadow) * (diffuse + specular);	
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
	vec3 ambient = light.ambient * texture(material.diffuse, fs_in.texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, fs_in.texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.specular, fs_in.texCoords).rgb;

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
	vec3 ambient = light.ambient * texture(material.diffuse, fs_in.texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, fs_in.texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.specular, fs_in.texCoords).rgb;

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
	vec3 ambient = light.ambient * texture(material.diffuse, fs_in.texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, fs_in.texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.specular, fs_in.texCoords).rgb;

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