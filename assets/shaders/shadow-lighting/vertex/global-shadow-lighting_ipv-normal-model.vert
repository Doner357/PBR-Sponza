#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
	vec4 dirLightSpacePos[1];
	vec4 spotLightSpacePos[2];
	mat3 TBN;
} vs_out;


uniform mat4 model;
uniform mat3 normalMat;
layout (std140) uniform CameraMatrices {
	mat4 view;
	mat4 projection;
};
layout (std140) uniform ShadowMatrices {
	mat4 dirLightSpaceMat[1];
	mat4 spotLightSpaceMat[2];
};

void main() {

	vec3 T = normalize(normalMat * aTangent);
	vec3 N = normalize(normalMat * aNormal);
	// Apply Gram-Schmidt process to re-orthogonalize the TBN matrix
	T = normalize(T - dot(T, N) * N);

	vec3 B = cross(N, T);
	vs_out.TBN = mat3(T, B, N);

	vs_out.texCoords = aTexCoords;
	vs_out.fragPos = vec3(model * vec4(aPos, 1.0));
	vs_out.normal =  normalMat * aNormal;
	vs_out.dirLightSpacePos[0] = dirLightSpaceMat[0] * vec4(vs_out.fragPos, 1.0);
	vs_out.spotLightSpacePos[0] = spotLightSpaceMat[0] * vec4(vs_out.fragPos, 1.0);
	vs_out.spotLightSpacePos[1] = spotLightSpaceMat[1] * vec4(vs_out.fragPos, 1.0);

	gl_Position = projection * view * model * vec4(aPos, 1.0);
}