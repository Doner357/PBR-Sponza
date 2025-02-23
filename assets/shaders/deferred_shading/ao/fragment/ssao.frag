#version 460 core
layout (location = 0) out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3  samples[64];
uniform int   kernelSize;   // The number of samples
uniform vec2  screenSize;   // The size of the screen, x component contains width, y contains height
uniform float radius;       // The radius of kernel
uniform float bias;         // The bias for sample depth
uniform float power;        // The power to control the strength of ao

// We need projection matrix
layout (std140) uniform CameraMatrices {
	mat4 view;
	mat4 projection;
};

void main() {
    // tile noise texture over screen, based on screen dimensions divided by noise size
    vec2 noise_scale = vec2(screenSize.x / 4.0, screenSize.y / 4.0); // screen = 800x600

    // Extract data from textures
    vec3 fragPos   = texture(gPosition, TexCoords).rgb;
    vec3 normal    = texture(gNormal, TexCoords).rgb;
    vec3 randomVec = texture(texNoise, TexCoords * noise_scale).rgb;

    // Using Gramm-Schmidt process to calculate tangent-space to view-sapce TBN matrix
    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);

    // ** AO calculation **
    float occlusion = 0.0;
    int num_of_samples = clamp(kernelSize, 0, 64);
    for (int i = 0; i < num_of_samples; i++) {
        // Get sample postion
        vec3 samplePos = TBN * samples[i]; // From tangent to view space
        samplePos = fragPos + samplePos * radius;

        // Transfer sample from view-space to screen-space
        vec4 offset = vec4(samplePos, 1.0);
        offset      = projection * offset;    // from view to clip-space
        offset.xyz /= offset.w;               // perspective divide
        offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

        // Sample depth
        float sample_depth = texture(gPosition, offset.xy).z;

        // Add contribution if the current fragment's depth is larger than sample depth
        // Check if the sample depth is too far away from current depth, 
        // if so, discard the contribution
        float range_check = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sample_depth));
        occlusion += (sample_depth >= samplePos.z + bias ? 1.0 : 0.0) * range_check;
    }

    // Normalize the occlusion contribution
    // Note that we subtract the occlusion factor from 1.0 so we can directly use the occlusion
    // factor to scale the ambient lighting component.
    occlusion = 1.0 - (occlusion / num_of_samples);
    FragColor = pow(occlusion, power);
}