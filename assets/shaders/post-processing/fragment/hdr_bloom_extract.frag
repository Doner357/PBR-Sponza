#version 410 core
// Multiple Render Targets
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    // Get the hdr color from screen textur
    vec3 scene_color = texture(screenTexture, TexCoords).rgb;

    // Paster regular scene color to regular attachment
    FragColor = vec4(scene_color, 1.0);
    
    // Check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(scene_color, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0) {
        BrightColor = vec4(scene_color, 1.0);
    }
    else {
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    }

}