#version 460 core
in vec2 TexCoords;

out vec4 FragColor;


uniform sampler2D gPosition;
uniform sampler2D gNormalShininess;
uniform sampler2D gAlbedoSpec;

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

// Calculate the radius of light volume (Demo), this actually doesn't save cost
float CalcLightRadius(vec3 light_color, float constant, float linear, float quadratic);	

uniform vec3 viewPos;
layout (std140) uniform GlobalLights {               // size      ali
	DirLight dirLights[NUM_OF_DIRLIGHTS];            //  256        0
	PointLight pointLights[NUM_OF_POINTLIGHTS];      // 2560	  256
	SpotLight spotLights[NUM_OF_SPOTLIGHTS];         //  768     2816
}; // total 3584


void main() {
	vec3 normal = normalize(texture(gNormalShininess, TexCoords).rgb);
	vec3 fragPos = texture(gPosition, TexCoords).rgb;
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 result = vec3(0.0);

	for(int i = 0; i < NUM_OF_DIRLIGHTS; i++) {
		result += dirLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcDirLight(dirLights[i], normal, viewDir);
	}
	for(int i = 0; i < NUM_OF_POINTLIGHTS; i++) {
		float distance = length(pointLights[i].position - fragPos);
		float radius = CalcLightRadius(pointLights[i].specular, pointLights[i].constant, pointLights[i].linear, pointLights[i].quadratic);
		if (distance < radius)
			result += pointLights[i].position == NO_LIGHT ? vec3(0.0) : CalcPointLight(pointLights[i], normal, fragPos, viewDir);
	}
	for(int i = 0; i < NUM_OF_SPOTLIGHTS; i++) {
		float distance = length(spotLights[i].position - fragPos);
		float radius = CalcLightRadius(spotLights[i].specular, spotLights[i].constant, spotLights[i].linear, spotLights[i].quadratic);
		if (distance < radius)
			result += spotLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcSpotLight(spotLights[i], normal, fragPos, viewDir);
	}
	
	FragColor = vec4(result, 1.0);
}

float CalcLightRadius(vec3 light_color, float constant, float linear, float quadratic) {
	float light_max = max(max(light_color.r, light_color.g), light_color.b);
	float radius = (-linear + sqrt(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * light_max))) / (2 * quadratic);
	return radius;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir){
	
	vec3 lightDir = normalize(-light.direction);
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	// --ambient--
	vec3 ambient = light.ambient * texture(gAlbedoSpec, TexCoords).rgb;

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
	vec3 ambient = light.ambient * texture(gAlbedoSpec, TexCoords).rgb;

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
	vec3 ambient = light.ambient * texture(gAlbedoSpec, TexCoords).rgb;

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