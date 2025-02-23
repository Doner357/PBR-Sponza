#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT {
	vec3 normal;
} vs_out;

uniform mat4 model;
uniform mat4 view;

void main() {
	gl_Position = view * model * vec4(aPos, 1.0);
	mat3 normalMat = mat3(inverse(transpose(view * model)));	// The normal matrix is calculated in Shader, which is not much different from the normal normal matrix. It is the view space normal matrix.
	vs_out.normal = normalize(vec3(vec4(normalMat * aNormal, 0.0)));
}