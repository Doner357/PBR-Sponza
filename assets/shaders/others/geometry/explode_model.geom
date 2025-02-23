#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
	vec2 texCoords;
	vec3 fragPos;
	vec3 normal;
} gs_in[];

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform float time;

vec3 GetNormal();								// Get the normal vector of the face
vec4 Explode(vec4 position, vec3 normal);		// Move a vertex along to given normal vector

void main()	{
	vec3 normal = GetNormal();

	gl_Position = Explode(gl_in[0].gl_Position, normal);
	TexCoords = gs_in[0].texCoords;
	FragPos = gs_in[0].fragPos;
	Normal = gs_in[0].normal;
	EmitVertex();
	gl_Position = Explode(gl_in[1].gl_Position, normal);
	TexCoords = gs_in[1].texCoords;
	FragPos = gs_in[1].fragPos;
	Normal = gs_in[1].normal;
	EmitVertex();
	gl_Position = Explode(gl_in[2].gl_Position, normal);
	TexCoords = gs_in[2].texCoords;
	FragPos = gs_in[2].fragPos;
	Normal = gs_in[2].normal;
	EmitVertex();

	EndPrimitive();
}

vec3 GetNormal() {
	vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);	// First vector origin at gl_in[1]
	vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);	// Second vector origin at gl_in[1]
	return normalize(cross(a, b));										// Calculate the normal vector
}

vec4 Explode(vec4 position, vec3 normal) {
	float magnitude = 2.0;
	vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude;
	return position + vec4(direction, 0.0);
}