#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

out vec2 vtexCoords;

uniform mat4 model;

void main() {
	gl_Position = model * vec4(aPos, 1.0);
	vtexCoords = aTexCoords;
}