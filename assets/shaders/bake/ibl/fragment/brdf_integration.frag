#version 410 core

out vec2 FragColor;

in vec2 TexCoords;

const float PI = 3.14159265359;

// Cook-Torrance BRDF functions
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float N_dot_V, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);

// Generate Hammersley Sequence
float RadicalInverse_VdC(uint bits);
vec2 Hammersley(uint i, uint N);

// Do importance sampling
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness);

vec2 IntegrateBRDF(float N_dot_V, float roughness);

void main() {
	vec2 integrated_brdf = IntegrateBRDF(TexCoords.x, TexCoords.y);
	FragColor = integrated_brdf;
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
	float a        = roughness * roughness;
	float a2       = a * a;
	float N_dot_H  = max(dot(N, H), 0.0);
	float N_dot_H2 = N_dot_H * N_dot_H;

	float num = a2;
	float denom = (N_dot_H2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float N_dot_V, float roughness) {
	float r = roughness;
	float k = (r * r) / 2.0;

	float nom   = N_dot_V;
	float denom = N_dot_V * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
	float N_dot_V = max(dot(N, V), 0.0);
	float N_dot_L = max(dot(N, L), 0.0);
	float ggx2    = GeometrySchlickGGX(N_dot_V, roughness);
	float ggx1    = GeometrySchlickGGX(N_dot_L, roughness);

	return ggx1 * ggx2;
}

float RadicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N) {
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cos_theta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    // From spherical coordinate to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sin_theta;
    H.y = sin(phi) * sin_theta;
    H.z = cos_theta;

    // From tangent-space vector to world-space vector
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sample_vec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sample_vec);
}

vec2 IntegrateBRDF(float N_dot_V, float roughness) {
	vec3 V;
	V.x = sqrt(1.0 - N_dot_V * N_dot_V);
	V.y = 0.0;
	V.z = N_dot_V;

	float A = 0.0;
	float B = 0.0;
	
	vec3 N = vec3(0.0, 0.0, 1.0);

	const uint kSampleCount = 8192u;
	for (uint i = 0u; i < kSampleCount; ++i) {
		vec2 Xi = Hammersley(i, kSampleCount);
		vec3 H  = ImportanceSampleGGX(Xi, N, roughness);
		vec3 L  = normalize(2.0 * dot(V, H) * H - V);
		
        float N_dot_L = max(L.z, 0.0);
        float N_dot_H = max(H.z, 0.0);
        float V_dot_H = max(dot(V, H), 0.0);

		if (N_dot_L > 0.0) {
			float G     = GeometrySmith(N, V, L, roughness);
			float G_Vis = (G * V_dot_H) / (N_dot_H * N_dot_V);
			float Fc    = pow(1.0 - V_dot_H, 5.0);

			A += (1.0 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	A /= float(kSampleCount);
	B /= float(kSampleCount);
	return vec2(A, B);
}