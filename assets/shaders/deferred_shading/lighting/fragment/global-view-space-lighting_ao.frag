#version 410 core
in vec2 TexCoords;

out vec4 FragColor;


uniform sampler2D gPosition;
uniform sampler2D gNormalShininess;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ambientOcclusion;

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
#define NUM_OF_DIRLIGHTS 4

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

// Lighting calculation function
//------------------------------
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);                     // Directional light calculation function
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);   // Point light calculation function
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);     // Spot light calculation function

layout (std140) uniform CameraMatrices {
	mat4 view;
	mat4 projection;
};
uniform mat3 directionMat;

layout (std140) uniform GlobalLights {               // size      ali
	DirLight dirLights[NUM_OF_DIRLIGHTS];            //  256        0
	PointLight pointLights[NUM_OF_POINTLIGHTS];      // 2560	  256
	SpotLight spotLights[NUM_OF_SPOTLIGHTS];         //  768     2816
}; // total 3584


void main() {
	vec3 normal = normalize(texture(gNormalShininess, TexCoords).rgb);
	vec3 fragPos = texture(gPosition, TexCoords).rgb;
	vec3 viewDir = normalize(vec3(0.0) - fragPos);
	vec3 result = vec3(0.0);

	for(int i = 0; i < NUM_OF_DIRLIGHTS; i++) {
		DirLight light = dirLights[i];
		if (light.direction != vec3(0.0)) {
			light.direction = directionMat * dirLights[i].direction;
			result += CalcDirLight(light, normal, viewDir);
		}
	}
	for(int i = 0; i < NUM_OF_POINTLIGHTS; i++) {
		PointLight light = pointLights[i];
		if (light.position != vec3(0.0)) {
			light.position = vec3((view * vec4(light.position, 1.0)).xyz);
			result += CalcPointLight(light, normal, fragPos, viewDir);
		}
	}
	for(int i = 0; i < NUM_OF_SPOTLIGHTS; i++) {
		SpotLight light = spotLights[i];
		if (light.position != vec3(0.0)) {
			light.position = vec3((view * vec4(light.position, 1.0)).xyz);
			light.direction = directionMat * light.direction;
			result += CalcSpotLight(light, normal, fragPos, viewDir);
		}
	}
	
	FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir){
	
	vec3 lightDir = normalize(-light.direction);
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	// --ambient--
	float ao = texture(ambientOcclusion, TexCoords).r;
	vec3 ambient = light.ambient * ao * texture(gAlbedoSpec, TexCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(gAlbedoSpec, TexCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), texture(gNormalShininess, TexCoords).a);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * vec3(texture(gAlbedoSpec, TexCoords).a);

	// --result--
	return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	
	vec3 lightDir = normalize(light.position - fragPos);
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	// --ambient--
	float ao = texture(ambientOcclusion, TexCoords).r;
	vec3 ambient = light.ambient * ao * texture(gAlbedoSpec, TexCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(gAlbedoSpec, TexCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), texture(gNormalShininess, TexCoords).a);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * vec3(texture(gAlbedoSpec, TexCoords).a);

	// --attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	// --result--
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	return ambient + diffuse + specular;

}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	
	vec3 lightDir = normalize(light.position - fragPos);
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	// --ambient--
	float ao = texture(ambientOcclusion, TexCoords).r;
	vec3 ambient = light.ambient * ao * texture(gAlbedoSpec, TexCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(gAlbedoSpec, TexCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), texture(gNormalShininess, TexCoords).a);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * vec3(texture(gAlbedoSpec, TexCoords).a);

	// --attenuation--
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	// --spotlight intensity--
	float theta     = dot(lightDir, normalize(-light.direction));                // the cosine of the angle between spotlight direction and light direction
	float epsilon   = light.innerCutOff - light.outerCutOff;                     // the cosine difference between the inner cone and outer cone
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);    // the intensity of spotlight

	// --result--
	return (ambient + (diffuse + specular) * intensity) * attenuation;
}