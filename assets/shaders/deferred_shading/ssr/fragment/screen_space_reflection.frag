#version 410 core
#define PI 3.14159265359

in VS_OUT {
	vec2 texCoords;
} fs_in;

layout (location = 0) out vec4 FragColor;

struct ScreenInfo {
    sampler2D  albedo;
	sampler2D  normal;
	sampler2D  material;
	sampler2D  position;
	sampler2D  scene;
};

// --environment light--
struct EnvLight {
	samplerCube irradiance;
	samplerCube prefiltered;
	sampler2D   preBrdf;
};


layout (std140) uniform CameraMatrices {
	mat4 view;
	mat4 projection;
};

uniform ScreenInfo screenInfo;

// Environment map
uniform EnvLight environment;

uniform float stepSize;
uniform float maxDistance;
uniform int   maxSteps;
uniform float hitThreshold;
uniform float reflectionStrength;

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


vec3 CalcLight(vec3 LightColor, float length, vec3 L, vec3 N, vec3 V, vec3 F0) {
	// Extract material attribution from texture
	float roughness = texture(screenInfo.material, fs_in.texCoords).g;
	float ao        = texture(screenInfo.material, fs_in.texCoords).b;

	roughness = max(roughness, 0.05);

	// Result for outgoing radiance
	vec3 Lo = vec3(0.0);

	L = normalize(L);

	// Half vector
	vec3 H = normalize(V + L);

	// light color
	float attenuation = 1.0 / length * length;
	vec3  radians = LightColor * attenuation;

	// Fresnel-Schlick
	vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);
	float R = pow(1.0 - roughness, 3);
	Lo += F * radians * R * ao;

	return Lo;
}


vec2 projectToScreen(vec3 viewPos) {
    vec4 clipPos = projection * vec4(viewPos, 1.0);
    clipPos.xyz /= clipPos.w;
    vec2 uv = clipPos.xy * 0.5 + 0.5;
    return uv;
}

// Check if coord is in [0, 1]
bool isInScreen(vec2 uv) {
    return (uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0);
}

void main() {
    vec3 albedo = texture(screenInfo.albedo, fs_in.texCoords).rgb;
    // Surface reflection at zero incidence
    float metallic = texture(screenInfo.material, fs_in.texCoords).r;
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 sceneColor = texture(screenInfo.scene, fs_in.texCoords).rgb;
    vec3 positionVS = texture(screenInfo.position, fs_in.texCoords).xyz;
    vec3 normalVS   = texture(screenInfo.normal, fs_in.texCoords).xyz;

    vec3 viewDir = normalize(-positionVS);

    normalVS = normalize(normalVS);
    vec3 reflDir = reflect(-viewDir, normalVS);

    vec3 rayPos = positionVS;
    vec3 hitColor = vec3(0.0);
    bool hit = false;

	float dis_step_size = stepSize;

    for (int i = 0; i < maxSteps; ++i) {

        rayPos += reflDir * dis_step_size;

        if (length(rayPos) > maxDistance)
            break;

        vec2 uv = projectToScreen(rayPos);
        if (!isInScreen(uv))
            break;
        
        vec3 scenePos = texture(screenInfo.position, uv).xyz;

        if (distance(scenePos, rayPos) < hitThreshold) {
            hitColor = texture(screenInfo.scene, uv).rgb;
            hit = true;
            break;
        }
    }

	float length = distance(positionVS, rayPos);

	FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    if (hit) {
        vec3 finalColor = CalcLight(hitColor, length, reflDir, normalVS, viewDir, F0);
        FragColor = vec4(finalColor, 1.0);
    }
}