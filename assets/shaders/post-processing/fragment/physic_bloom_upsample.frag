#version 460 core
out vec4 upsample;

in vec2 TexCoords;

uniform sampler2D image;
uniform float filterRadius;

void main() {
    float x = filterRadius;
    float y = filterRadius;

    vec3 a = texture(image, vec2(TexCoords.x - x, TexCoords.y + y)).rgb;
    vec3 b = texture(image, vec2(TexCoords.x,     TexCoords.y + y)).rgb;
    vec3 c = texture(image, vec2(TexCoords.x + x, TexCoords.y + y)).rgb;

    vec3 d = texture(image, vec2(TexCoords.x - x, TexCoords.y)).rgb;
    vec3 e = texture(image, vec2(TexCoords.x,     TexCoords.y)).rgb;
    vec3 f = texture(image, vec2(TexCoords.x + x, TexCoords.y)).rgb;

    vec3 g = texture(image, vec2(TexCoords.x - x, TexCoords.y - y)).rgb;
    vec3 h = texture(image, vec2(TexCoords.x,     TexCoords.y - y)).rgb;
    vec3 i = texture(image, vec2(TexCoords.x + x, TexCoords.y - y)).rgb;

    vec3 upsample_temp = e*4.0;
    upsample_temp += (b+d+f+h)*2.0;
    upsample_temp += (a+c+g+i);
    upsample_temp *= 1.0 / 16.0;
    upsample = vec4(upsample_temp, 1.0f);
}