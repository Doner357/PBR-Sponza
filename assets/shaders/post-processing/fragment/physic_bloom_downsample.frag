#version 460 core
out vec4 downsample;

in vec2 TexCoords;

uniform sampler2D image;
uniform vec2 imgResolution;
uniform int mipLevel;

vec3 PowVec3(vec3 v, float p) {
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float invGamma = 1.0 / 2.2;
vec3 ToSRGB(vec3 v) { return PowVec3(v, invGamma); }

float RGBToLuminance(vec3 col) {
    return dot(col, vec3(0.2126f, 0.7152f, 0.0722f));
}

float KarisAverage(vec3 col) {
    // Formula is 1 / (1 + luma)
    float luma = RGBToLuminance(ToSRGB(col)) * 0.25f;
    return 1.0f / (1.0f + luma);
}

void main() {
    vec2 img_tex_size = 1.0 / imgResolution;
    float x = img_tex_size.x;
    float y = img_tex_size.y;

    vec3 a = texture(image, vec2(TexCoords.x - 2 * x, TexCoords.y + 2 * y)).rgb;
    vec3 b = texture(image, vec2(TexCoords.x        , TexCoords.y + 2 * y)).rgb;
    vec3 c = texture(image, vec2(TexCoords.x + 2 * x, TexCoords.y + 2 * y)).rgb;
    
    vec3 d = texture(image, vec2(TexCoords.x - 2 * x, TexCoords.y)).rgb;
    vec3 e = texture(image, vec2(TexCoords.x        , TexCoords.y)).rgb;
    vec3 f = texture(image, vec2(TexCoords.x + 2 * x, TexCoords.y)).rgb;
    
    vec3 g = texture(image, vec2(TexCoords.x - 2 * x, TexCoords.y - 2 * y)).rgb;
    vec3 h = texture(image, vec2(TexCoords.x        , TexCoords.y - 2 * y)).rgb;
    vec3 i = texture(image, vec2(TexCoords.x + 2 * x, TexCoords.y - 2 * y)).rgb;
    
    vec3 j = texture(image, vec2(TexCoords.x - x, TexCoords.y + y)).rgb;
    vec3 k = texture(image, vec2(TexCoords.x + x, TexCoords.y + y)).rgb;
    vec3 l = texture(image, vec2(TexCoords.x - x, TexCoords.y - y)).rgb;
    vec3 m = texture(image, vec2(TexCoords.x + x, TexCoords.y - y)).rgb;

    vec3 groups[5];
    vec3 downsample_temp = vec3(0.0);
    switch (mipLevel) {
    case 0:
        // We are writing to mip 0, so we need to apply Karis average to each block
        // of 4 samples to prevent fireflies (very bright subpixels, leads to pulsating
        // artifacts).
        groups[0] = (a + b + d + e) * (0.125f / 4.0f);
        groups[1] = (b + c + e + f) * (0.125f / 4.0f);
        groups[2] = (d + e + g + h) * (0.125f / 4.0f);
        groups[3] = (e + f + h + i) * (0.125f / 4.0f);
        groups[4] = (j + k + l + m) * (0.5f / 4.0f);
        groups[0] *= KarisAverage(groups[0]);
        groups[1] *= KarisAverage(groups[1]);
        groups[2] *= KarisAverage(groups[2]);
        groups[3] *= KarisAverage(groups[3]);
        groups[4] *= KarisAverage(groups[4]);
        downsample_temp = groups[0] + groups[1] + groups[2] + groups[3] + groups[4];
        break;
    default:
        downsample_temp   = e * 0.125;
        downsample_temp += (a + c + g + i) * 0.03125;
        downsample_temp += (b + d + f + h) * 0.0625;
        downsample_temp += (j + k + l + m) * 0.125;
        break;
    }

    downsample_temp = max(downsample_temp, 0.0001f);
    downsample = vec4(downsample_temp, 1.0);
}