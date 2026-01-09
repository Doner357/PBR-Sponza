#version 410 core

in VS_OUT {
	vec2 texCoords;
	vec3 fragPos;
	vec3 normal;
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


// --point light with shadow--
struct ShadowPointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	samplerCube shadowMap;

	float far_plane;    // far plane of perspective projection
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
float PointLightShadowCalculation(ShadowPointLight light, vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);    // Calculate shadow factor

// Lighting calculation function
//------------------------------
vec3 CalcShadowPointLight(ShadowPointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);    // Point light with shadow caculation function
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);                                  // Directional light calculation function
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);                // Point light calculation function
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);                  // Spot light calculation function


// Uniform
//------------------------------
uniform Material material;                            // Material attributes
uniform vec3 viewPos;                                 // Viewer position
uniform DirLight dirLights[NUM_OF_DIRLIGHTS];         // Directional light
uniform PointLight pointLights[NUM_OF_POINTLIGHTS];   // Point light
uniform SpotLight spotLights[NUM_OF_SPOTLIGHTS];      // Spot light

uniform ShadowPointLight pointLight_sh;               // point light with shadow


void main() {
	// Properties
	vec3 normal = normalize(fs_in.normal);
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	vec3 result = vec3(0.0);

	result += pointLight_sh.position == NO_LIGHT ? vec3(0.0) : CalcShadowPointLight(pointLight_sh, normal, fs_in.fragPos, viewDir);

	for(int i = 0; i < NUM_OF_DIRLIGHTS; i++)
		result += dirLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcDirLight(dirLights[i], normal, viewDir);
	for(int i = 0; i < NUM_OF_POINTLIGHTS; i++)
		result += pointLights[i].position == NO_LIGHT ? vec3(0.0) : CalcPointLight(pointLights[i], normal, fs_in.fragPos, viewDir);
	for(int i = 0; i < NUM_OF_SPOTLIGHTS; i++)
		result += spotLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcSpotLight(spotLights[i], normal, fs_in.fragPos, viewDir);

	float alpha = texture(material.diffuse, fs_in.texCoords).a;
	FragColor = vec4(result, alpha);
}

float PointLightShadowCalculation(ShadowPointLight light, vec3 fragPos, vec3 normal) {
	// Get the vector from light to fragment position
	vec3 fragToLight = fragPos - light.position;
	// Get the depth value from current fragment to light source
	float currentDepth = length(fragToLight);

	// Apply PCF (Percentage-closer filtering)
	// This is too costly
	/*
	float shadow  = 0.0;
	float bias    = 0.055 - 0.05 * dot(normal, normalize(fragToLight));    // 0.005 + (0.05 * (1 - dot(normal, normalize(fragToLight))))
	float samples = 4.0;
	float offset  = 0.1;
	for (float x = -offset; x < offset; x += offset / (samples * 0.5)) {
		for (float y = -offset; y < offset; y += offset / (samples * 0.5)) {
			for (float z = -offset; z < offset; z += offset / (samples * 0.5)) {
				// Get the closest depth perspect from light
				float closestDepth = texture(light.shadowMap, fragToLight + vec3(x, y, z)).r;
				// Trans the light's depth from [0, 1] back to [0, far_plane]
				closestDepth *= light.far_plane;
				if (currentDepth - bias > closestDepth)
					shadow += 1.0;
			}
		}
	}
	shadow /= samples * samples * samples;
	*/

	// It is too costly to sample 4 * 4 * 4 = 64 samples, instead we create an offset array to cheap the sampling
	float shadow  = 0.0;
	float bias    = 0.02 - 0.015 * dot(normal, normalize(fragToLight));    // 0.005 + (0.015 * (1 - dot(normal, normalize(fragToLight))))
	int samples   = 20;
	float viewDistance = length(viewPos - fragPos);
	// Change radius based on the distance of the viewer to the fragment,
	// making the shadows softer when far away and sharper when close by.
	float diskRadius = (1.0 + (viewDistance / light.far_plane)) / 25.0;
	vec3 sampleOffsetDirections[20] = vec3[](
		vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
		vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
		vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
		vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
		vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	);
	for (int i = 0; i < samples; i++) {
		// Get the closest depth perspect from light
		float closestDepth = texture(light.shadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		// Trans the light's depth from [0, 1] back to [0, far_plane]
		closestDepth *= light.far_plane;
		if (currentDepth - bias > closestDepth)
			shadow += 1.0;
	}
	shadow /= float(samples);

	return shadow;
}

vec3 CalcShadowPointLight(ShadowPointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {

	// Basic data calculation
	//--------------------------
	vec3 lightDir = normalize(light.position - fragPos);   // light's direction
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

	// --shadow--
	float shadow = PointLightShadowCalculation(light, fragPos, normal);

	// Result
	//--------------------------
	return ambient + (1.0 - shadow) * (diffuse + specular);	
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