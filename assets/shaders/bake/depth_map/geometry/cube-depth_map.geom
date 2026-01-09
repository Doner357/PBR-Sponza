#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

in vec2 vtexCoords[];

out vec4 FragPos;   // FragPos from GS (output per emitvertex)
out vec2 gtexCoords;

uniform mat4 shadowMatrices[6];
uniform int layer;

void main() {
	for (int face = 0; face < 6; face++) {
		gl_Layer = layer * 6 + face;    // built-in variable that specifies to which face we render.
		for (int i = 0; i < 3; i++) {    // for each triangle vertex
			FragPos = gl_in[i].gl_Position;
			gtexCoords = vtexCoords[i];
			gl_Position = shadowMatrices[face] * FragPos;
			EmitVertex();
		}
		EndPrimitive();
	}
}