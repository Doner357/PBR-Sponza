#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
	vec3 fragPos;     // Fragment position
	vec3 normal;      // Normal
	vec2 texCoords;   // Texture coordinate
} vs_out;


uniform mat4 model;
layout (std140) uniform CameraMatrices {
	mat4 view;
	mat4 projection;
};
uniform mat3 normalMat;
uniform int repeat_times;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	
	vs_out.texCoords = aTexCoords * repeat_times;
	vs_out.fragPos = vec3(model * vec4(aPos, 1.0));
	vs_out.normal =  normalMat * aNormal;
}