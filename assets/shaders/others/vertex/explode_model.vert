#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
	vec2 texCoords;
	vec3 fragPos;
	vec3 normal;
} vs_out;

uniform mat4 model;
layout (std140) uniform CameraMatrices {
	mat4 view;
	mat4 projection;
};
uniform mat3 normalMat;

void main() {
	vs_out.texCoords = aTexCoords;
	vs_out.fragPos = (model * vec4(aPos, 1.0)).xyz;
	vs_out.normal = normalMat * aNormal;

	gl_Position = projection * view * model * vec4(aPos, 1.0);
}