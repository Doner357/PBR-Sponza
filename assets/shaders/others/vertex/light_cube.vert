#version 410 core
layout (location = 0) in vec3 aPos;

out vec3 norm;

uniform mat4 model;
layout (std140) uniform CameraMatrices {
	mat4 view;
	mat4 projection;
};

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	norm = normalize(mat3(model) * aPos);
}