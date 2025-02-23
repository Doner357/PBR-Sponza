#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

// Used for gamma correction
layout (std140) uniform GammaCorrection {
    float gamma;    // 4 bytes
};

// Conversion matrices and constants
const mat3 LINEAR_REC2020_TO_LINEAR_SRGB = mat3(
  1.6605, -0.1246, -0.0182,
  -0.5876, 1.1329, -0.1006,
  -0.0728, -0.0083, 1.1187
);

const mat3 LINEAR_SRGB_TO_LINEAR_REC2020 = mat3(
  0.6274, 0.0691, 0.0164,
  0.3293, 0.9195, 0.0880,
  0.0433, 0.0113, 0.8956
);

const mat3 AgXInsetMatrix = mat3(
  0.856627153315983, 0.137318972929847, 0.11189821299995,
  0.0951212405381588, 0.761241990602591, 0.0767994186031903,
  0.0482516061458583, 0.101439036467562, 0.811302368396859
);

const mat3 AgXOutsetMatrix = mat3(
  1.1271005818144368, -0.1413297634984383, -0.14132976349843826,
  -0.11060664309660323, 1.157823702216272, -0.11060664309660294,
  -0.016493938717834573, -0.016493938717834257, 1.2519364065950405
);

const float AgxMinEv = -18.47393;
const float AgxMaxEv = 3.526069;

vec3 agxAscCdl(vec3 color, vec3 slope, vec3 offset, vec3 power, float sat) {
  const vec3 lw = vec3(0.2126, 0.7152, 0.0722);
  float luma = dot(color, lw);
  vec3 c = pow(color * slope + offset, power);
  return luma + sat * (c - luma);
}

vec3 agx(vec3 color) {
    color = LINEAR_SRGB_TO_LINEAR_REC2020 * color;

    // Input transform (inset)
    color = AgXInsetMatrix * color;

    color = max(color, 1e-10); // Avoid 0 or negative numbers for log2

    // Log2 space encoding
    color = clamp(log2(color), AgxMinEv, AgxMaxEv);
    color = (color - AgxMinEv) / (AgxMaxEv - AgxMinEv);

    color = clamp(color, 0.0, 1.0);

    // Apply sigmoid function approximation
    vec3 x2 = color * color;
    vec3 x4 = x2 * x2;
    color = + 15.5     * x4 * x2
            - 40.14    * x4 * color
            + 31.96    * x4
            - 6.868    * x2 * color
            + 0.4298   * x2
            + 0.1191   * color
            - 0.00232;

    // Punchy
    color = agxAscCdl(color, vec3(1.0), vec3(0.0), vec3(1.35), 1.4);

    // Inverse input transform (outset)
    color = AgXOutsetMatrix * color;

    // sRGB IEC 61966-2-1 2.2 Exponent Reference EOTF Display
    color = pow(max(vec3(0.0), color), vec3(gamma));

    color = LINEAR_REC2020_TO_LINEAR_SRGB * color;
    color = clamp(color, 0.0, 1.0);

    return color;
}

void main() {
    // Get the hdr color from screen texture
    vec3 hdr_color = texture(screenTexture, TexCoords).rgb;

    // Apply AgX tone mapping
    vec3 mapped = agx(hdr_color);

    // Gamma correction
    //mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
}
