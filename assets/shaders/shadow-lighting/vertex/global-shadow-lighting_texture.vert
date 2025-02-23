#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
	vec4 dirLightSpacePos[1];
	vec4 spotLightSpacePos[2];
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
	vs_out.texCoords = aTexCoords;
	vs_out.fragPos = vec3(model * vec4(aPos, 1.0));
	vs_out.normal =  normalMat * aNormal;
	vs_out.dirLightSpacePos[0] = dirLightSpaceMat[0] * vec4(vs_out.fragPos, 1.0);
	vs_out.spotLightSpacePos[0] = spotLightSpaceMat[0] * vec4(vs_out.fragPos, 1.0);
	vs_out.spotLightSpacePos[1] = spotLightSpaceMat[1] * vec4(vs_out.fragPos, 1.0);

	gl_Position = projection * view * model * vec4(aPos, 1.0);
}