#version 410 core

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	float shininess;
};

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

// Lighting calculation function
//------------------------------
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);                     // Directional light calculation function

uniform vec3 ViewPos;
uniform Material material;
uniform DirLight dirLight;

// Used for gamma correction
layout (std140) uniform GammaCorrection {
	float gamma;    // 4 bytes
};

void main() {
	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(ViewPos - FragPos);

	vec3 result = CalcDirLight(dirLight, normal, viewDir);

	// Gamma correction
	float gam = gamma == 0.0 ? 1.0 : gamma;    // Avoid the 0 exponent
	result = pow(result, vec3(1.0 / gam));
	
	float alpha = texture(material.texture_diffuse1, TexCoords).a;
	FragColor = vec4(result, alpha);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir){
	
	vec3 lightDir = normalize(-light.direction);

	// --ambient--
	vec3 ambient = light.ambient * texture(material.texture_diffuse1, TexCoords).rgb;

	// --diffuse--
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, TexCoords).rgb;

	// --specular--
	vec3 reflectDir = normalize(reflect(-lightDir, normal));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.texture_specular1, TexCoords).rgb;

	// --result--
	return ambient + diffuse + specular;
}