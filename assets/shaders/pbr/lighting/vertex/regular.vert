#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoords;

out VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
	mat3 TBN;
	mat3 inverse_TBN;
} vs_out;

uniform mat4 model;
layout (std140) uniform CameraMatrices {
	mat4 view;
	mat4 projection;
};
uniform mat3 normalMat;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);

	vs_out.texCoords = aTexCoords;
	vs_out.fragPos = vec3(model * vec4(aPos, 1.0));
	vs_out.normal =  normalMat * aNormal;

	vec3 T = normalize(normalMat * aTangent);
	vec3 B = normalize(normalMat * aBitangent);
	vec3 N = normalize(normalMat * aNormal);

	mat3 TBN = mat3(T, B, N);
	vs_out.TBN = TBN;
	vs_out.inverse_TBN = transpose(TBN);
}