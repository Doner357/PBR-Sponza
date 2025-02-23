#version 460 core
out vec4 FragColor;
in  vec2 TexCoords;

uniform sampler2D screenTexture;

////////////////////////////////////////////////////////////////////////////////
// Check if two colors are "similar enough" to be considered the same color.
////////////////////////////////////////////////////////////////////////////////
bool isSameColor(vec3 c1, vec3 c2, float epsilon)
{
    return distance(c1, c2) < epsilon;
}

void main()
{
    // 1) Obtain texture size to compute per-texel offsets
    ivec2 texSize   = textureSize(screenTexture, 0);
    vec2  texelSize = 1.0 / vec2(texSize);

    // 2) Read the 3×3 (9) samples around the current fragment
    vec3 colors[9];
    int  index = 0;
    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            // Offset in UV space
            vec2 uvOffset  = vec2(i, j) * texelSize;
            vec2 sampleUV  = TexCoords + uvOffset;
            colors[index]  = texture(screenTexture, sampleUV).rgb;
            index++;
        }
    }

    // 3) Compare each color with the others in the 9-sample array
    //    to find the "dominant color" that has the maximum count.
    int   maxCount       = -1;
    vec3  dominantColor  = vec3(0.0);
    float epsilon        = 0.02; // threshold for "same color"

    for(int i = 0; i < 9; i++)
    {
        vec3 c1 = colors[i];
        int count = 0;

        // Count how many samples are "similar" to c1
        for(int j = 0; j < 9; j++)
        {
            if(isSameColor(c1, colors[j], epsilon))
            {
                count++;
            }
        }

        // If this color has a higher count, update the dominant color
        if(count > maxCount)
        {
            maxCount      = count;
            dominantColor = c1;
        }
    }

    // 4) Output the dominant color among the 3×3 neighborhood
    FragColor = vec4(dominantColor, 1.0);
}
