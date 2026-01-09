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

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	
	vs_out.texCoords = aTexCoords;
	vs_out.fragPos = vec3(model * vec4(aPos, 1.0));

	/* The matrix calculation below is normal matrix, which can convert normal vector to model space.
	Inversing matrices is a costly operation for shaders, You'd better calculate this in
	CPU and send it to the shaders via a uniform before drawing. */
	//Normal = mat3(transpose(inverse(model))) * aNormal;

	vs_out.normal =  normalMat * aNormal;   // Get normal matrix from CPU
}