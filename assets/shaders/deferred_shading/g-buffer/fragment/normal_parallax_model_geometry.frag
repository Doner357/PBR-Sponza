#version 410 core

in VS_OUT {
    vec3 fragPos;
    vec2 texCoords;
    mat3 TBN;
	mat3 inverse_TBN;
} fs_in;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

// Properties struct
//------------------------------
struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_normal1;
	sampler2D texture_height1;
	float shininess;
};

uniform Material material;
uniform vec3 viewPos;
uniform float height_scale;

// Steep Parallax mapping function
//------------------------------
vec2 ParallaxOcclusionMapping(vec2 texCoords, vec3 viewDir);


void main() {
	// calculate basic data
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	viewDir = normalize(fs_in.inverse_TBN * viewDir);
	// Offset the texture coordinates
	vec2 texcoord = ParallaxOcclusionMapping(fs_in.texCoords, viewDir);

	// Store the fragment position vector in the first gbuffer texture
	gPosition = fs_in.fragPos;
	// also store the per-fragment normals into the gbuffer
	vec3 normal = texture(material.texture_normal1, texcoord).rgb;
	normal = normal * 2.0 - 1.0;
	gNormal   = fs_in.TBN * normal;
	// and the diffuse per-fragment color
	gAlbedoSpec.rgb = texture(material.texture_diffuse1, texcoord).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
	gAlbedoSpec.a   = texture(material.texture_specular1, texcoord).r;
}

vec2 ParallaxOcclusionMapping(vec2 texCoords, vec3 viewDir) {
	// Number of layer
	const float kMinLayers = 8.0;
	const float kMaxLayers = 32.0;
	// Taking less samples when looking straight at a surface and more samples when looking at an angle
	// Note that the normal vector is equal to z-axis in tangent space
	float kNumOfLayers = mix(kMaxLayers, kMinLayers, min(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));

	// Calculate the size of each layer
	float layer_depth = 1.0 / kNumOfLayers;
	// depth of current layer
	float current_layer_depth = 0.0;
	// The amount to shift the texture coordinates per layer (from vector p)
	vec2 p = viewDir.xy * height_scale;
	vec2 delta_texture_coords = p / kNumOfLayers;

	// Get initial values
	vec2 current_texture_coords = texCoords;
	float current_depth_map_value = texture(material.texture_height1, current_texture_coords).r;
	
	// Iterate until the sample depth value is less than current depth value
	while (current_layer_depth < current_depth_map_value) {
		// Shift texture coordinates along direction of p
		current_texture_coords -= delta_texture_coords;
		// Get depthmap value at current texture coordinates
		current_depth_map_value = texture(material.texture_height1, current_texture_coords).r;
		// Get depth of next layer
		current_layer_depth += layer_depth;
	}


	/*
		Do parallax occlusion mapping
	*/
	// Get previous texture coordinate
	vec2 prev_texuture_coords = current_texture_coords + delta_texture_coords;

	// Get depth after and before collision for linear interpolation
	float after_depth = current_depth_map_value - current_layer_depth;
	float before_depth = texture(material.texture_height1, prev_texuture_coords).r - current_layer_depth + layer_depth;

	// Calculate the weight
	float weight = after_depth / (after_depth - before_depth);

	// Calculate the final texture coordinate
	vec2 final_texture_coords = (prev_texuture_coords * weight) + (current_texture_coords * (1.0 - weight));

	return final_texture_coords;
}