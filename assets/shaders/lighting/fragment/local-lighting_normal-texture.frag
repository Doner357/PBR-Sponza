/*
    This is a simple fragment shader allowing one point light source.
*/
#version 460 core

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
    vec3 tangentLightPos;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
} fs_in;

out vec4 FragColor;

layout (std140) uniform GammaCorrection {
	float gamma;    // 4 bytes
};


struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    float shininess;
};

struct Light {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light pointlight;

// Calculate the light result
vec3 CalcLight(Light light, vec3 lightDir, vec3 viewDir, vec3 norm);

void main() {

    vec3 norm = texture(material.normal, fs_in.texCoords).rgb;
    norm = normalize(norm * 2.0 - 1.0);

    vec3 result = vec3(0.0);
    vec3 lightDir = normalize(fs_in.tangentLightPos - fs_in.tangentFragPos);
    vec3 viewDir = normalize(fs_in.tangentViewPos - fs_in.tangentFragPos);
    result += CalcLight(pointlight, lightDir, viewDir, norm);
    
	// Gamma correction
	float gam = gamma == 0.0 ? 1.0 : gamma;    // Avoid the 0 exponent
	result = pow(result, vec3(1.0 / gam));

    float alpha = texture(material.diffuse, fs_in.texCoords).a;
    FragColor = vec4(result, alpha);
}

vec3 CalcLight(Light pointlight, vec3 lightDir, vec3 viewDir, vec3 norm) {
    vec3 halfwayDir = normalize(viewDir + lightDir);

    // ambient
    vec3 ambient = pointlight.ambient * texture(material.diffuse, fs_in.texCoords).rgb;

    // diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = pointlight.diffuse * diff * texture(material.diffuse, fs_in.texCoords).rgb;

    // specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    spec = diff != 0.0 ? spec : 0.0;
    vec3 specular = pointlight.specular * spec * texture(material.specular, fs_in.texCoords).rgb;

    // attenuation
    float distance = length(fs_in.tangentFragPos - fs_in.tangentLightPos);
    float attenuation = 1.0 / pow(distance, 2.0);

    // result
    return (ambient + diffuse + specular) * attenuation;

}