#version 460 core

in VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
	vec4 dirLightSpacePos[1];
	vec4 spotLightSpacePos[2];
	mat3 inverse_TBN;
} fs_in;

out vec4 FragColor;


// Properties struct
//------------------------------
// --material--
struct Material {
	//vec3 ambient;
	sampler2D diffuse;
	sampler2D specular;
	sampler2D normal;
	sampler2D parallax;
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

// Steep Parallax mapping function
//------------------------------
vec2 ParallaxOcclusionMapping(vec2 texCoords, vec3 viewDir);

// Lighting calculation function
//------------------------------
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoords);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords);
vec3 CalcShadowDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoords, int shadow_id);
vec3 CalcShadowPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords, int shadow_id);
vec3 CalcShadowSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords, int shadow_id);


// Uniform
//------------------------------
uniform Material material;
uniform vec3 viewPos;

// Value to control the height scale
uniform float height_scale;

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
	// View direction
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	// Transfer view direction to tangent sapce
	viewDir = normalize(fs_in.inverse_TBN * viewDir);
	
	// Displace the texture coordinate
	vec2 texCoords = ParallaxOcclusionMapping(fs_in.texCoords, viewDir);
	// If sample outside [0, 1] range, discard the pixel
	if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
		discard;
	
	// Get normal vector
	vec3 normal = texture(material.normal, texCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	// Initialize the result color
	vec3 result = vec3(0.0);

	// Light wiht no shadow
	for(int i = 0; i < NUM_OF_DIRLIGHTS; i++)
		result += dirLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcDirLight(dirLights[i], normal, viewDir, texCoords);
	for(int i = 0; i < NUM_OF_POINTLIGHTS; i++)
		result += pointLights[i].position == NO_LIGHT ? vec3(0.0) : CalcPointLight(pointLights[i], normal, fs_in.fragPos, viewDir, texCoords);
	for(int i = 0; i < NUM_OF_SPOTLIGHTS; i++)
		result += spotLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcSpotLight(spotLights[i], normal, fs_in.fragPos, viewDir, texCoords);

	// Light with shadow
	for (int i = 0; i < NUM_OF_SHADOWDIRLIGHTS; i++)
		result += shadowDirLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcShadowDirLight(shadowDirLights[i], normal, viewDir, texCoords, i);
	for (int i = 0; i < NUM_OF_SHADOWPOINTLIGHTS; i++)
		result += shadowPointLights[i].position == NO_LIGHT ? vec3(0.0) : CalcShadowPointLight(shadowPointLights[i], normal, fs_in.fragPos, viewDir, texCoords, i);
	for (int i = 0; i < NUM_OF_SHADOWSPOTLIGHTS; i++)
		result += shadowSpotLights[i].direction == NO_LIGHT ? vec3(0.0) : CalcShadowSpotLight(shadowSpotLights[i], normal, fs_in.fragPos, viewDir, texCoords, i);

	float alpha = texture(material.diffuse, fs_in.texCoords).a;
	FragColor = vec4(result, alpha);
}

vec2 ParallaxOcclusionMapping(vec2 texCoords, vec3 viewDir) {
	// Number of layer
	const float kMinLayers = 8.0;
	const float kMaxLayers = 32.0;
	// Taking less samples when looking straight at a surface and more samples when looking at an angle
	// Note that the normal vector is equal to z-axis in tangent space
	float kNumOfLayers = mix(kMaxLayers, kMinLayers, min(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));

	// Calculate the size of each layer
	float layer_depth = 1.0 / kNumOfLayers;
	// depth of current layer
	float current_layer_depth = 0.0;
	// The amount to shift the texture coordinates per layer (from vector p)
	vec2 p = viewDir.xy * height_scale;
	vec2 delta_texture_coords = p / kNumOfLayers;

	// Get initial values
	vec2 current_texture_coords = texCoords;
	float current_depth_map_value = texture(material.parallax, current_texture_coords).r;
	
	// Iterate until the sample depth value is less than current depth value
	while (current_layer_depth < current_depth_map_value) {
		// Shift texture coordinates along direction of p
		current_texture_coords -= delta_texture_coords;
		// Get depthmap value at current texture coordinates
		current_depth_map_value = texture(material.parallax, current_texture_coords).r;
		// Get depth of next layer
		current_layer_depth += layer_depth;
	}


	/*
		Do parallax occlusion mapping
	*/
	// Get previous texture coordinate
	vec2 prev_texuture_coords = current_texture_coords + delta_texture_coords;

	// Get depth after and before collision for linear interpolation
	float after_depth = current_depth_map_value - current_layer_depth;
	float before_depth = texture(material.parallax, prev_texuture_coords).r - current_layer_depth + layer_depth;

	// Calculate the weight
	float weight = after_depth / (after_depth - before_depth);

	// Calculate the final texture coordinate
	vec2 final_texture_coords = (prev_texuture_coords * weight) + (current_texture_coords * (1.0 - weight));

	return final_texture_coords;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoords) {

	// Basic data calculation
	//--------------------------
	// Transfer lighit direction to tangent space for proper lighting calculation
	vec3 lightDir = normalize(fs_in.inverse_TBN * -light.direction);   // light's direction
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	// Lighting Calculation
	//--------------------------
	// --ambient--
	vec3 ambient = light.ambient * texture(material.diffuse, texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.specular, texCoords).rgb;

	// Result
	//--------------------------
	return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords) {
	// Basic data calculation
	//--------------------------
	// Transfer lighit direction to tangent space for proper lighting calculation
	vec3 lightDir = normalize(fs_in.inverse_TBN *(light.position - fragPos));   // light direction
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	// Lighting Calculation
	//--------------------------
	// --ambient--
	vec3 ambient = light.ambient * texture(material.diffuse, texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.specular, texCoords).rgb;

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

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords) {
	// Basic data calculation
	//--------------------------
	// Transfer lighit direction to tangent space for proper lighting calculation
	vec3 lightDir = normalize(fs_in.inverse_TBN * (light.position - fragPos));        // light direction
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	
	// Lighting Calculation
	//--------------------------
	// --ambient--
	vec3 ambient = light.ambient * texture(material.diffuse, texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.specular, texCoords).rgb;

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

vec3 CalcShadowDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoords, int shadow_id) {

	// Basic data calculation
	//--------------------------
	// Transfer lighit direction to tangent space for proper lighting calculation
	vec3 lightDir = normalize(fs_in.inverse_TBN * -light.direction);   // light's direction
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	// Lighting Calculation
	//--------------------------
	// --ambient--
	vec3 ambient = light.ambient * texture(material.diffuse, texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.specular, texCoords).rgb;

	// --shadow--
	float shadow = DirLightShadowCalculation(light, fs_in.dirLightSpacePos[shadow_id], normal, lightDir, shadow_id);

	// Result
	//--------------------------
	return ambient + (1 - shadow) * (diffuse + specular);	
}

vec3 CalcShadowPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords, int shadow_id) {

	// Basic data calculation
	//--------------------------
	// Transfer lighit direction to tangent space for proper lighting calculation
	vec3 lightDir = normalize(fs_in.inverse_TBN * (light.position - fragPos));   // light's direction
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	// Lighting Calculation
	//--------------------------
	// --ambient--
	vec3 ambient = light.ambient * texture(material.diffuse, texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.specular, texCoords).rgb;
	
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

vec3 CalcShadowSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords, int shadow_id) {
	// Basic data calculation
	//--------------------------
	// Transfer lighit direction to tangent space for proper lighting calculation
	vec3 lightDir = normalize(fs_in.inverse_TBN * (light.position - fragPos));        // light direction
	// Blinn-Phong halfway direction factor
	vec3 halfwayDir = normalize(viewDir + lightDir);

	
	// Lighting Calculation
	//--------------------------
	// --ambient--
	vec3 ambient = light.ambient * texture(material.diffuse, texCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoords).rgb;

	// --specular--
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);     // Measure the angle between normal and halfway instead of view direction and refection direction
	spec = diff != 0.0 ? spec : 0.0;                                             // Avoid some light specular light error when the light is below the surface
	vec3 specular = light.specular * spec * texture(material.specular, texCoords).rgb;

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