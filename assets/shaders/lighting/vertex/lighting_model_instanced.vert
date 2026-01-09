#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;

out VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
} vs_out;

uniform mat4 model;
layout (std140) uniform CameraMatrices {
	mat4 view;
	mat4 projection;
};
//uniform mat3 normalMat;

void main() {
	vs_out.texCoords = aTexCoords;
	vs_out.fragPos = (model * instanceMatrix * vec4(aPos, 1.0)).xyz;
	mat3 normalMat = mat3(inverse(transpose(model * instanceMatrix)));	// Calculate normal matrix in shader since it is difficult to set up in main program.
	vs_out.normal = normalMat * aNormal;

	gl_Position = projection * view * model * instanceMatrix * vec4(aPos, 1.0);
}