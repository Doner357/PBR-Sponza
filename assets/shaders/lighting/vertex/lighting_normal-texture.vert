#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// The vertex shader's output datas
out VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
    vec3 tangentLightPos;       // The light position which has been transfered into tangent space
    vec3 tangentViewPos;        // The view position which has been transfered into tangent space
    vec3 tangentFragPos;        // The fragment position which has been transfered into tangent space
} vs_out;

uniform mat4 model;
uniform mat3 normalMat;
layout (std140) uniform CameraMatrices {
    mat4 view;
    mat4 projection;
};

uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    // Regular tangent, bitangent construct
	vec3 T = normalize(normalMat * aTangent);
	vec3 B = normalize(normalMat * aBitangent);
	vec3 N = normalize(normalMat * aNormal);

    // Basic lighting datas output
    vs_out.fragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.texCoords = aTexCoords;
    vs_out.normal = normalMat * aNormal;

    // Inverse the TBN matrix so it can transfer coordinate from world space to tangent space
    mat3 TBN = transpose(mat3(T, B, N));
    // Apply inverse TBN to basic lighting datas to transfer them into tangent space
    vs_out.tangentLightPos = TBN * lightPos;
    vs_out.tangentViewPos  = TBN * viewPos;
    vs_out.tangentFragPos  = TBN * vec3(model * vec4(aPos, 1.0));

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}