#version 460 core

out vec4 FragColor;

in  vec3 localPos;

uniform samplerCube environment_map;

const float PI = 3.14159265359;

void main() {

    // The sample direction is equals the hemisphere's orientation
    vec3 normal = normalize(localPos);

    vec3 irradiance = vec3(0.0, 0.0, 0.0);

    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, normal));
    up         = normalize(cross(normal, right));

    const float kSampleDelta = 0.007;
    float num_samples = 0.0;
    modf((1.0 * PI * PI) / (kSampleDelta * kSampleDelta), num_samples);

    for (float phi = 0.0; phi < 2.0 * PI; phi += kSampleDelta) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += kSampleDelta) {
            // Spherical to cartesian (in tangent space)
            vec3 tangent_sample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // Tangent space to world
            vec3 sample_vector = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * normal;

            irradiance += texture(environment_map, sample_vector).rgb * cos(theta) * sin(theta) / num_samples;
        }
    }

    irradiance = PI * irradiance;
    FragColor = vec4(irradiance, 1.0);
}