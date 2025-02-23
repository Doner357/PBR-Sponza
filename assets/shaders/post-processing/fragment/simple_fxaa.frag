#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture; // The rendered scene

// You can adjust these constants as needed
const float FXAA_SPAN_MAX   = 8.0;    // Maximum search steps in either direction
const float FXAA_REDUCE_MIN = 1.0/128.0;
const float FXAA_REDUCE_MUL = 1.0/8.0;

void main()
{
    // Fetch the color of the current pixel
    vec3 rgbM  = texture(screenTexture, TexCoords).rgb;

    // Calculate inverse texture size for sampling neighboring pixels
    vec2 texSize = vec2(textureSize(screenTexture, 0));
    vec2 rcpTex  = 1.0 / texSize;

    // Sample the four diagonal neighbors (NW, NE, SW, SE)
    vec3 rgbNW = texture(screenTexture, TexCoords + vec2(-rcpTex.x, -rcpTex.y)).rgb;
    vec3 rgbNE = texture(screenTexture, TexCoords + vec2( rcpTex.x, -rcpTex.y)).rgb;
    vec3 rgbSW = texture(screenTexture, TexCoords + vec2(-rcpTex.x,  rcpTex.y)).rgb;
    vec3 rgbSE = texture(screenTexture, TexCoords + vec2( rcpTex.x,  rcpTex.y)).rgb;

    // Compute luminance (basic luma, assuming sRGB/BT.601-like weights)
    float lumM  = dot(rgbM,  vec3(0.299, 0.587, 0.114));
    float lumNW = dot(rgbNW, vec3(0.299, 0.587, 0.114));
    float lumNE = dot(rgbNE, vec3(0.299, 0.587, 0.114));
    float lumSW = dot(rgbSW, vec3(0.299, 0.587, 0.114));
    float lumSE = dot(rgbSE, vec3(0.299, 0.587, 0.114));

    // Find minimum and maximum luminance within this area
    float lumMin = min(lumM, min(min(lumNW, lumNE), min(lumSW, lumSE)));
    float lumMax = max(lumM, max(max(lumNW, lumNE), max(lumSW, lumSE)));

    // If the lum range is small, there's no strong edge
    float lumRange = lumMax - lumMin;
    if(lumRange < FXAA_REDUCE_MIN) {
        FragColor = vec4(rgbM, 1.0);
        return;
    }

    // Compute the mean lum of corners and a multiplier for edge detection
    float lumMean  = (lumNW + lumNE + lumSW + lumSE) * 0.25;
    float rangeMul = clamp(lumRange * FXAA_REDUCE_MUL, 0.0, 1.0);

    // Estimate an edge direction (rough gradient)
    vec2 blurDir  = vec2(
        -((lumNW + lumNE) - (lumSW + lumSE)),
         ((lumNW + lumSW) - (lumNE + lumSE))
    ) * 0.25;

    // Normalize the direction vector if it's not zero
    float dist = length(blurDir);
    if(dist > 0.0) {
        blurDir /= dist;
    }

    // Limit the maximum extent
    blurDir = clamp(blurDir * FXAA_SPAN_MAX, -rcpTex * 8.0, rcpTex * 8.0);

    // Take two samples along the estimated edge to blend
    vec3 rgb1 = texture(screenTexture, TexCoords + blurDir * (1.0/3.0) * rangeMul).rgb;
    vec3 rgb2 = texture(screenTexture, TexCoords + blurDir * (2.0/3.0) * rangeMul).rgb;

    // Combine these two samples
    vec3 fxaaResult = 0.5 * (rgb1 + rgb2);

    // Final output
    FragColor = vec4(fxaaResult, 1.0);
}
