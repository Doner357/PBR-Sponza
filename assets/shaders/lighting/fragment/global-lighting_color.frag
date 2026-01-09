#version 410 core
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float shininess;
};

struct DirLight {       // size  ali
	vec3 direction;     // 12    0

	vec3 ambient;       // 12    12 + 4 = 16
	vec3 diffuse;       // 12    28 + 4 = 32
	vec3 specular;      // 12    44 + 4 = 48
}; // total 60 + 4 = 64 bytes
#define NUM_OF_DIRLIGHTS 4

struct PointLight {		// size  ali
	vec3 position;		// 12    0

	float constant;		//  4    12
	float linear;       //  4    16
	float quadratic;    //  4    20
	
	vec3 ambient;       // 12    24 + 8 = 32
	vec3 diffuse;       // 12    44 + 4 = 48
	vec3 specular;      // 12    60 + 4 = 64
}; // total 76 + 4 = 80 bytes
#define NUM_OF_POINTLIGHTS 32

struct SpotLight {      // size  ali
	vec3 position;      // 12    0
	vec3 direction;     // 12    12 + 4 = 16

	float innerCutOff;  //  4    28
	float outerCutOff;  //  4    32

	float constant;     //  4    36
	float linear;       //  4    40
	float quadratic;    //  4    44
	
	vec3 ambient;       // 12    48
	vec3 diffuse;       // 12    60 + 4 = 64
	vec3 specular;      // 12    76 + 4 = 80
}; // total 92 + 4 = 96 bytes
#define NUM_OF_SPOTLIGHTS 8

#define NO_LIGHT vec3(0.0)


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);


uniform vec3 viewPos;
uniform Material material;
layout (std140) uniform GlobalLights {               // size      ali
	DirLight dirLights[NUM_OF_DIRLIGHTS];            //  256        0
	PointLight pointLights[NUM_OF_POINTLIGHTS];      // 2560	  256
	SpotLight spotLights[NUM_OF_SPOTLIGHTS];         //  768     2816
}; // total 3584

void main() {
	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 result = vec3(0.0);
	
	for(int i = 0; i < NUM_OF_DIRLIGHTS; i++)
		result += dirLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcDirLight(dirLights[i], normal, viewDir);
	for(int i = 0; i < NUM_OF_POINTLIGHTS; i++)
		result += pointLights[i].position == NO_LIGHT ? vec3(0.0) : CalcPointLight(pointLights[i], normal, FragPos, viewDir);
	for(int i = 0; i < NUM_OF_SPOTLIGHTS; i++)
		result += spotLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcSpotLight(spotLights[i], normal, FragPos, viewDir);

	FragColor = vec4(result, 1.0);
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
	vec3 ambient = light.ambient * material.ambient;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * material.diffuse;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * material.specular;

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
	vec3 ambient = light.ambient * material.ambient;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * material.diffuse;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * material.specular;

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
	vec3 ambient = light.ambient * material.ambient;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * material.diffuse;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * material.specular;

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