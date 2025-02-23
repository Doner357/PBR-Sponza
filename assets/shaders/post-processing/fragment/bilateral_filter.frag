#version 460 core

out vec4 FragColor;
in  vec2 TexCoords;

// The precomputed SSR result texture.
// Typically, this texture contains SSR colors (or black for misses).
uniform sampler2D screenTexture;

// Bilateral filter parameters
uniform float sigmaS;  // Spatial sigma for distance-based Gaussian attenuation
uniform float sigmaR;  // Range sigma for color-based Gaussian attenuation
uniform float radius;  // Filter radius (e.g., 2.0 for 5x5, 3.0 for 7x7, etc.)

// Texture size (width, height) of screenTexture
// Alternatively, you could call textureSize(screenTexture, 0) in the shader
uniform vec2 texSize;  

//------------------------------------------
// Gaussian function
//------------------------------------------
float gauss(float x, float sigma)
{
    // Gauss function: exp(-x^2 / (2 * sigma^2))
    return exp( -(x * x) / (2.0 * sigma * sigma) );
}

void main()
{
    // (0) Fetch the center pixel's SSR color
    vec3 centerColor = texture(screenTexture, TexCoords).rgb;

    // (1) Compute texel size to iterate over neighboring pixels
    vec2 invSize = 1.0 / texSize;

    // (2) Initialize accumulators
    vec3  finalColor  = vec3(0.0);
    float totalWeight = 0.0;

    // (3) Perform bilateral filtering within [ -radius, +radius ] 
    int r = int(radius);
    for(int i = -r; i <= r; i++)
    {
        for(int j = -r; j <= r; j++)
        {
            // (a) Compute neighbor UV
            vec2 offset  = vec2(float(i), float(j)) * invSize;
            vec2 sampleUV= TexCoords + offset;

            // (b) Sample SSR color
            vec3 sampleColor = texture(screenTexture, sampleUV).rgb;

            // (c) Spatial weight: Gaussian attenuation based on pixel distance
            float dist2   = float(i * i + j * j); // i^2 + j^2
            float wSpace  = gauss(dist2, sigmaS);

            // (d) Color weight: Gaussian attenuation based on color difference
            float colorDist2 = distance(sampleColor, centerColor);
            float wColor     = gauss(colorDist2, sigmaR);

            // (e) Combined weight
            float weight = wSpace * wColor;

            // (f) Accumulate the filtered color
            finalColor  += sampleColor * weight;
            totalWeight += weight;
        }
    }

    // (4) Normalize the final color
    vec3 denoisedColor = (totalWeight > 0.0) 
                       ? (finalColor / totalWeight)
                       : centerColor;

    // (5) Output the denoised SSR color
    FragColor = vec4(denoisedColor, 1.0);
}
