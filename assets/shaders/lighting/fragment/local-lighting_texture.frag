#version 460 core

in VS_OUT {
	vec3 fragPos;     // Fragment position
	vec3 normal;      // Fragment normal(denormalized)
	vec2 texCoords;   // Texture coordinate
} fs_in;


out vec4 FragColor;

// Properties struct
//------------------------------
// --material--
struct Material {
	//vec3 ambient;      // This is same as diffuse color, we don't need this now since we get it from diffuse color
	sampler2D diffuse;   // Get the ambient and diffuse colors from texture called diffuse map
	sampler2D specular;  // Get specular color from specular map
	float shininess;     // shininess value
};

// --direction light--
struct DirLight {
	vec3 direction;      // light's direction

	vec3 ambient;        // ambient  light's color & intensity
	vec3 diffuse;        // diffuse  light's color & intensity
	vec3 specular;       // specular light's color & intensity
};
#define NUM_OF_DIRLIGHTS 4

// --point light--
struct PointLight {
	vec3 position;       // light's position

	float constant;      // constant for attenuation
	float linear;        // linear for attenuation
	float quadratic;     // quadratic for attenuation

	vec3 ambient;        // ambient  light's color & intensity
	vec3 diffuse;        // diffuse  light's color & intensity
	vec3 specular;       // specular light's color & intensity
};
#define NUM_OF_POINTLIGHTS 32   // Use pre-processor directive in GLSL to define the number of point lights

struct SpotLight {
	vec3 position;       // light's position
	vec3 direction;      // light's direction

	float innerCutOff;   // inner cone cosine cut off for spot light's soft edges
	float outerCutOff;   // outer cone cosine cut off for spot light's soft edges
	
	float constant;      // constant for attenuation
	float linear;        // linear for attenuation
	float quadratic;     // quadratic for attenuation

	vec3 ambient;        // ambient  light's color & intensity
	vec3 diffuse;        // diffuse  light's color & intensity
	vec3 specular;       // specular light's color & intensity
};
#define NUM_OF_SPOTLIGHTS 8

#define NO_LIGHT vec3(0.0)


// Lighting calculation function
//------------------------------
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);                     // Directional light calculation function
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);   // Point light calculation function
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);     // Spot light calculation function


// Uniform
//------------------------------
uniform Material material;                            // Material attributes
uniform vec3 viewPos;                                 // Viewer position
uniform DirLight dirLights[NUM_OF_DIRLIGHTS];         // Directional light
uniform PointLight pointLights[NUM_OF_POINTLIGHTS];   // Point light
uniform SpotLight spotLights[NUM_OF_SPOTLIGHTS];      // Spot light


void main() {
	// Properties
	vec3 normal = normalize(fs_in.normal);
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	vec3 result = vec3(0.0);

	// Phase 1: Directional lighting
	for(int i = 0; i < NUM_OF_DIRLIGHTS; i++)
		result += dirLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcDirLight(dirLights[i], normal, viewDir);
	// Phase 2: Point lights
	for(int i = 0; i < NUM_OF_POINTLIGHTS; i++)
		result += pointLights[i].position == NO_LIGHT ? vec3(0.0) : CalcPointLight(pointLights[i], normal, fs_in.fragPos, viewDir);
	// Phase 3: Spot light
	for(int i = 0; i < NUM_OF_SPOTLIGHTS; i++)
		result += spotLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcSpotLight(spotLights[i], normal, fs_in.fragPos, viewDir);

	float alpha = texture(material.diffuse, fs_in.texCoords).a;
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