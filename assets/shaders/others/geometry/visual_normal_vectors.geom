#version 410 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;		// Since one triangle has three points, to generate line for each point, there are six points needed
in VS_OUT {
	vec3 normal;
} gs_in[];

const float MAGNITUDE = 0.4;

uniform mat4 projection;

void GenerateLine(int index);

void main() {
	for (int i = 0; i < 3; i++)		// Generate three normal vectors
		GenerateLine(i);
}

void GenerateLine(int index) {
	gl_Position = projection * gl_in[index].gl_Position;
	EmitVertex();
	gl_Position = projection * (gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * MAGNITUDE);
	EmitVertex();

	EndPrimitive();
}