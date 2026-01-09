#version 410 core

in VS_OUT {
    vec3 fragPos;
	vec3 normal;
    vec2 texCoords;
} fs_in;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormalShininess;
layout (location = 2) out vec4 gAlbedoSpec;


void main() {
	// Store the fragment position vector in the first gbuffer texture
	gPosition = fs_in.fragPos;
	// also store the per-fragment normals into the gbuffer
	gNormalShininess.rgb = normalize(fs_in.normal);
	// Put shininess into alpha channel of normal buffer
	gNormalShininess.a   = 64.0;
	// and the fixed diffuse per-fragment color
	gAlbedoSpec.rgb = vec3(0.95);
    // store fixed specular intensity in gAlbedoSpec's alpha component
	gAlbedoSpec.a = 0.1;
}