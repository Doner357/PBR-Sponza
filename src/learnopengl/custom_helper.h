#ifndef COMMON_VAO_CREATOR_H
#define COMMON_VAO_CREATOR_H

#include <glad/glad.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "shader_m.h"
#include "camera_plus.h"

#include <iostream>
#include <random>
#include <ctime>
#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace CustomHelper {

    enum VAOType {
        VAO_CUBE,
        VAO_ROOM,
        VAO_QUAD,
        VAO_SKYBOX,
        VAO_SPHERE
    };

    enum UBOBindingPoints {
        UBOPOINT_CAMERA_MATRICES = 0,
        UBOPOINT_BLINPHONG_LIGHTING = 1,
        UBOPOINT_GAMMA_CORRECTION = 2,
        UBOPOINT_BLINPHONG_SHADOWLIGHTING = 3,
        UBOPOINT_BLINPHONG_SHADOWMATRICES = 4,
        UBOPOINT_BLINPHONG_SHADOWFARPLANE = 5
    };
    extern std::string UBOPOINT_NAME_CAMERA_MATRICES;
    extern std::string UBOPOINT_NAME_BLINPHONG_LIGHTING;
    extern std::string UBOPOINT_NAME_GAMMA_CORRECTION;
    extern std::string UBOPOINT_NAME_BLINPHONG_SHADOWLIGHTING;
    extern std::string UBOPOINT_NAME_BLINPHONG_SHADOWMATRICES;
    extern std::string UBOPOINT_NAME_BLINPHONG_SHADOWFARPLANE;
    
    enum LightType {
        DIRECTIONAL_LIGHT,
        POINT_LIGHT,
        SPOT_LIGHT
    };

    enum LightProperties {
        MAX_NUM_DIRECTIONALLIGHT = 4,
        MAX_NUM_POINTLIGHT = 32,
        MAX_NUM_SPOTLIGHT = 8,
        MAX_NUM_SHADOWDIRECTIONALLIGHT = 4,
        MAX_NUM_SHADOWPOINTLIGHT = 16,
        MAX_NUM_SHADOWSPOTLIGHT = 16
    };

    enum ShaderSamplerNum {
        SAMPLER_DIFFUSE = 0,
        SAMPLER_SPECULAR = 1,
        SAMPLER_NORMAL = 2,
        SAMPLER_HEIGHT = 3,
        SAMPLER_DIRSHADOW0 = 10,
        SAMPLER_POINTSHADOW0 = 11,
        SAMPLER_SPOTSHADOW0 = 12,
    };

    const float cubeVertices[] = {
        // positions            // normal                // texture Coords
        // Back face
        -1.0f, -1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    0.0f, 0.0f, // Bottom-left
         1.0f,  1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    1.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    1.0f, 0.0f, // bottom-right
         1.0f,  1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    1.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    0.0f, 0.0f, // bottom-left
        -1.0f,  1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    0.0f, 1.0f, // top-left
        // Front face
        -1.0f, -1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    0.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    1.0f, 0.0f, // bottom-right
         1.0f,  1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    1.0f, 1.0f, // top-right
         1.0f,  1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    0.0f, 0.0f, // bottom-left
        // Left face
        -1.0f,  1.0f,  1.0f,    -1.0f,  0.0f,  0.0f,    1.0f, 0.0f, // top-right
        -1.0f,  1.0f, -1.0f,    -1.0f,  0.0f,  0.0f,    1.0f, 1.0f, // top-left
        -1.0f, -1.0f, -1.0f,    -1.0f,  0.0f,  0.0f,    0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f,    -1.0f,  0.0f,  0.0f,    0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,    -1.0f,  0.0f,  0.0f,    0.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f,    -1.0f,  0.0f,  0.0f,    1.0f, 0.0f, // top-right
        // Right face
         1.0f,  1.0f,  1.0f,     1.0f,  0.0f,  0.0f,    1.0f, 0.0f, // top-left
         1.0f, -1.0f, -1.0f,     1.0f,  0.0f,  0.0f,    0.0f, 1.0f, // bottom-right
         1.0f,  1.0f, -1.0f,     1.0f,  0.0f,  0.0f,    1.0f, 1.0f, // top-right         
         1.0f, -1.0f, -1.0f,     1.0f,  0.0f,  0.0f,    0.0f, 1.0f, // bottom-right
         1.0f,  1.0f,  1.0f,     1.0f,  0.0f,  0.0f,    1.0f, 0.0f, // top-left
         1.0f, -1.0f,  1.0f,     1.0f,  0.0f,  0.0f,    0.0f, 0.0f, // bottom-left     
         // Bottom face
        -1.0f, -1.0f, -1.0f,     0.0f, -1.0f,  0.0f,    0.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,     0.0f, -1.0f,  0.0f,    1.0f, 1.0f, // top-left
         1.0f, -1.0f,  1.0f,     0.0f, -1.0f,  0.0f,    1.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,     0.0f, -1.0f,  0.0f,    1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,     0.0f, -1.0f,  0.0f,    0.0f, 0.0f, // bottom-right
        -1.0f, -1.0f, -1.0f,     0.0f, -1.0f,  0.0f,    0.0f, 1.0f, // top-right
         // Top face
        -1.0f,  1.0f, -1.0f,     0.0f,  1.0f,  0.0f,    0.0f, 1.0f, // top-left
         1.0f,  1.0f,  1.0f,     0.0f,  1.0f,  0.0f,    1.0f, 0.0f, // bottom-right
         1.0f,  1.0f, -1.0f,     0.0f,  1.0f,  0.0f,    1.0f, 1.0f, // top-right     
         1.0f,  1.0f,  1.0f,     0.0f,  1.0f,  0.0f,    1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f, -1.0f,     0.0f,  1.0f,  0.0f,    0.0f, 1.0f, // top-left
        -1.0f,  1.0f,  1.0f,     0.0f,  1.0f,  0.0f,    0.0f, 0.0f  // bottom-left
    };

    const float roomVertices[] = {
        // positions            // normal                // texture Coords
        // Back face
         1.0f,  1.0f, -1.0f,     0.0f,  0.0f,  1.0f,    1.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,     0.0f,  0.0f,  1.0f,    0.0f, 0.0f, // Bottom-left
         1.0f, -1.0f, -1.0f,     0.0f,  0.0f,  1.0f,    1.0f, 0.0f, // bottom-right
        -1.0f, -1.0f, -1.0f,     0.0f,  0.0f,  1.0f,    0.0f, 0.0f, // bottom-left
         1.0f,  1.0f, -1.0f,     0.0f,  0.0f,  1.0f,    1.0f, 1.0f, // top-right
        -1.0f,  1.0f, -1.0f,     0.0f,  0.0f,  1.0f,    0.0f, 1.0f, // top-left
        // Front face
         1.0f, -1.0f,  1.0f,     0.0f,  0.0f, -1.0f,    1.0f, 0.0f, // bottom-right
        -1.0f, -1.0f,  1.0f,     0.0f,  0.0f, -1.0f,    0.0f, 0.0f, // bottom-left
         1.0f,  1.0f,  1.0f,     0.0f,  0.0f, -1.0f,    1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,     0.0f,  0.0f, -1.0f,    0.0f, 1.0f, // top-left
         1.0f,  1.0f,  1.0f,     0.0f,  0.0f, -1.0f,    1.0f, 1.0f, // top-right
        -1.0f, -1.0f,  1.0f,     0.0f,  0.0f, -1.0f,    0.0f, 0.0f, // bottom-left
        // Left face
        -1.0f,  1.0f, -1.0f,     1.0f,  0.0f,  0.0f,    1.0f, 1.0f, // top-left
        -1.0f,  1.0f,  1.0f,     1.0f,  0.0f,  0.0f,    1.0f, 0.0f, // top-right
        -1.0f, -1.0f, -1.0f,     1.0f,  0.0f,  0.0f,    0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,     1.0f,  0.0f,  0.0f,    0.0f, 0.0f, // bottom-right
        -1.0f, -1.0f, -1.0f,     1.0f,  0.0f,  0.0f,    0.0f, 1.0f, // bottom-left
        -1.0f,  1.0f,  1.0f,     1.0f,  0.0f,  0.0f,    1.0f, 0.0f, // top-right
        // Right face
         1.0f, -1.0f, -1.0f,    -1.0f,  0.0f,  0.0f,    0.0f, 1.0f, // bottom-right
         1.0f,  1.0f,  1.0f,    -1.0f,  0.0f,  0.0f,    1.0f, 0.0f, // top-left
         1.0f,  1.0f, -1.0f,    -1.0f,  0.0f,  0.0f,    1.0f, 1.0f, // top-right
         1.0f,  1.0f,  1.0f,    -1.0f,  0.0f,  0.0f,    1.0f, 0.0f, // top-left
         1.0f, -1.0f, -1.0f,    -1.0f,  0.0f,  0.0f,    0.0f, 1.0f, // bottom-right
         1.0f, -1.0f,  1.0f,    -1.0f,  0.0f,  0.0f,    0.0f, 0.0f, // bottom-left     
         // Bottom face
         1.0f, -1.0f, -1.0f,     0.0f,  1.0f,  0.0f,    1.0f, 1.0f, // top-left
        -1.0f, -1.0f, -1.0f,     0.0f,  1.0f,  0.0f,    0.0f, 1.0f, // top-right
         1.0f, -1.0f,  1.0f,     0.0f,  1.0f,  0.0f,    1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,     0.0f,  1.0f,  0.0f,    0.0f, 0.0f, // bottom-right
         1.0f, -1.0f,  1.0f,     0.0f,  1.0f,  0.0f,    1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f, -1.0f,     0.0f,  1.0f,  0.0f,    0.0f, 1.0f, // top-right
         // Top face
         1.0f,  1.0f,  1.0f,     0.0f, -1.0f,  0.0f,    1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f, -1.0f,     0.0f, -1.0f,  0.0f,    0.0f, 1.0f, // top-left
         1.0f,  1.0f, -1.0f,     0.0f, -1.0f,  0.0f,    1.0f, 1.0f, // top-right
        -1.0f,  1.0f, -1.0f,     0.0f, -1.0f,  0.0f,    0.0f, 1.0f, // top-left
         1.0f,  1.0f,  1.0f,     0.0f, -1.0f,  0.0f,    1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f,     0.0f, -1.0f,  0.0f,    0.0f, 0.0f  // bottom-left
    };

    const float quadVertices[] = {
        // positions            // normal                // texture Coords
         1.0f,  1.0f,  0.0f,     0.0f,  0.0f,  1.0f,    1.0f,  1.0f,
        -1.0f,  1.0f,  0.0f,     0.0f,  0.0f,  1.0f,    0.0f,  1.0f,
        -1.0f, -1.0f,  0.0f,     0.0f,  0.0f,  1.0f,    0.0f,  0.0f,

         1.0f, -1.0f,  0.0f,     0.0f,  0.0f,  1.0f,    1.0f,  0.0f,
         1.0f,  1.0f,  0.0f,     0.0f,  0.0f,  1.0f,    1.0f,  1.0f,
        -1.0f, -1.0f,  0.0f,     0.0f,  0.0f,  1.0f,    0.0f,  0.0f
    };

    const float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    class VAOManager {
        public:
            VAOManager() : sphere_index_count(0) {
                InitialCubeVAO();
                InitialRoomVAO();
                InitialQuadVAO();
                InitialSkyboxVAO();
                InitialShereVAO();
            };
            ~VAOManager() {
                this->clean();
            }
            unsigned int getVAO(VAOType type) {
                std::string VAOname;
                switch (type) {
                case VAO_CUBE:
                    VAOname = "Common_VAO_Cube";
                    break;
                case VAO_ROOM:
                    VAOname = "Common_VAO_Room";
                    break;
                case VAO_QUAD:
                    VAOname = "Common_VAO_Quad";
                    break;
                case VAO_SKYBOX:
                    VAOname = "Common_VAO_Skybox";
                    break;
                case VAO_SPHERE:
                    VAOname = "Common_VAO_Sphere";
                    break;
                default:
                    break;
                }

                return findVAO(VAOname);
            }

            unsigned int getVAO(const std::string name) {
                return findVAO(name);
            }

            void registerVAO(const std::string name, const unsigned int VAO) {
                this->storedVAO[name] = VAO;
            }

            void clean() {
                for (auto it = this->storedVAO.begin(); it != this->storedVAO.end(); it++)
                    glDeleteVertexArrays(1, &(it->second));
            }

            size_t getSphereIndexCount() {
                return this->sphere_index_count;
            }
        private:
            std::map<std::string, unsigned int> storedVAO;
            size_t sphere_index_count;
            
            void InitialCubeVAO() {
                unsigned int VAO, VBO;
                glGenVertexArrays(1, &VAO);
                glGenBuffers(1, &VBO);
                glBindVertexArray(VAO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices[0], GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
                glBindVertexArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glDeleteBuffers(1, &VBO);

                this->registerVAO("Common_VAO_Cube", VAO);
            }
            void InitialRoomVAO() {
                unsigned int VAO, VBO;
                glGenVertexArrays(1, &VAO);
                glGenBuffers(1, &VBO);
                glBindVertexArray(VAO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(roomVertices), &roomVertices[0], GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
                glBindVertexArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glDeleteBuffers(1, &VBO);

                this->registerVAO("Common_VAO_Room", VAO);
            }
            void InitialQuadVAO() {
                unsigned int VAO, VBO;
                glGenVertexArrays(1, &VAO);
                glGenBuffers(1, &VBO);
                glBindVertexArray(VAO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
                glBindVertexArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glDeleteBuffers(1, &VBO);

                this->registerVAO("Common_VAO_Quad", VAO);
            }
            void InitialSkyboxVAO() {
                unsigned int VAO, VBO;
                glGenVertexArrays(1, &VAO);
                glGenBuffers(1, &VBO);
                glBindVertexArray(VAO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices[0], GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                glBindVertexArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glDeleteBuffers(1, &VBO);

                this->registerVAO("Common_VAO_Skybox", VAO);
            }
            void InitialShereVAO() {
                unsigned int sphereVAO = 0;
                unsigned int indexCount;

                glGenVertexArrays(1, &sphereVAO);

                unsigned int vbo, ebo;
                glGenBuffers(1, &vbo);
                glGenBuffers(1, &ebo);

                std::vector<glm::vec3> positions;
                std::vector<glm::vec2> uv;
                std::vector<glm::vec3> normals;
                std::vector<glm::vec3> tangents;
                std::vector<glm::vec3> bitangents;
                std::vector<unsigned int> indices;

                const unsigned int X_SEGMENTS = 32;
                const unsigned int Y_SEGMENTS = 32;
                const float PI = 3.14159265359f;
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
                    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
                        float xOffset  = 1.0f / static_cast<float>(X_SEGMENTS);
                        float yOffset  = 1.0f / static_cast<float>(Y_SEGMENTS);
                        float xSegment = xOffset * static_cast<float>(x);
                        float ySegment = yOffset * static_cast<float>(y);
                        glm::vec3 position;
                        position.x = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                        position.y = std::cos(ySegment * PI);
                        position.z = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                        // Normal, Tangent and Bitangent calculation algorithm from :
                        // https://computergraphics.stackexchange.com/questions/5498/compute-sphere-tangent-for-normal-mapping
                        // Space axis used to cross center to position vector
                        glm::vec3 axis = glm::vec3(0.0f, 1.0f, 0.0f);
                        // The vector point from center of the sphere to the vertex's position
                        glm::vec3 center_to_position = glm::normalize(position - glm::vec3(0.0f, 0.0f, 0.0f));
                        // Handle exception (center to position vector points up or down)
                        if (y == 0) {
                            center_to_position.x = std::cos(xSegment * 2.0f * PI) * std::sin((ySegment + yOffset) * PI);
                            center_to_position.y = std::cos((ySegment + yOffset) * PI);
                            center_to_position.z = std::sin(xSegment * 2.0f * PI) * std::sin((ySegment + yOffset) * PI);
                        }
                        else if (y == Y_SEGMENTS) {
                            center_to_position.x = std::cos(xSegment * 2.0f * PI) * std::sin((ySegment - yOffset) * PI);
                            center_to_position.y = std::cos((ySegment - yOffset) * PI);
                            center_to_position.z = std::sin(xSegment * 2.0f * PI) * std::sin((ySegment - yOffset) * PI);
                        }
                        // In this sphere, normal vector equal to vertex position
                        glm::vec3 normal = glm::normalize(position);
                        glm::vec3 tangent = glm::cross(axis, center_to_position);
                        glm::vec3 bitangent = glm::cross(normal, tangent);

                        // How many time should texture repeate?
                        const unsigned int x_repeate_time = 2;
                        const unsigned int y_repeate_time = 1;

                        positions.push_back(position);
                        uv.push_back(glm::vec2((1.0f - xSegment) * x_repeate_time, (1.0f - ySegment) * y_repeate_time));
                        normals.push_back(position);
                        tangents.push_back(tangent);
                        bitangents.push_back(bitangent);
                    }
                }


                bool oddRow = false;
                for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
                    if (!oddRow) // even rows: y == 0, y == 2; and so on
                    {
                        for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
                            indices.push_back(y * (X_SEGMENTS + 1) + x);
                            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                        }
                    }
                    else {
                        for (int x = X_SEGMENTS; x >= 0; --x) {
                            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                            indices.push_back(y * (X_SEGMENTS + 1) + x);
                        }
                    }
                    oddRow = !oddRow;
                }
                indexCount = static_cast<unsigned int>(indices.size());

                std::vector<float> data;
                for (unsigned int i = 0; i < positions.size(); ++i) {
                    data.push_back(positions[i].x);
                    data.push_back(positions[i].y);
                    data.push_back(positions[i].z);
                    if (normals.size() > 0) {
                        data.push_back(normals[i].x);
                        data.push_back(normals[i].y);
                        data.push_back(normals[i].z);
                    }
                    if (uv.size() > 0) {
                        data.push_back(uv[i].x);
                        data.push_back(uv[i].y);
                    }
                    if (tangents.size() > 0) {
                        data.push_back(tangents[i].x);
                        data.push_back(tangents[i].y);
                        data.push_back(tangents[i].z);
                    }
                    if (bitangents.size() > 0) {
                        data.push_back(bitangents[i].x);
                        data.push_back(bitangents[i].y);
                        data.push_back(bitangents[i].z);
                    }
                }
                glBindVertexArray(sphereVAO);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
                unsigned int stride = (3 + 2 + 3 + 3 + 3) * sizeof(float);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float)));
                glEnableVertexAttribArray(3);
                glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void *)(8 * sizeof(float)));
                glEnableVertexAttribArray(4);
                glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void *)(11 * sizeof(float)));

                this->sphere_index_count = indexCount;
                this->registerVAO("Common_VAO_Sphere", sphereVAO);
            }

            unsigned int findVAO(const std::string name) {
                unsigned int target = 0;
                std::map<std::string, unsigned int>::iterator it = this->storedVAO.find(name);
                if (it != this->storedVAO.end())
                    target = it->second;

                return target;
            }
    };


    template<typename T>
    class UniformBufferManager {
        
        private:
            std::string      uniform_block_name;
            unsigned int     uniform_buffer;
            UBOBindingPoints binding_point;
            GLsizeiptr       size_of_total_block;

            std::vector<T> elements;


        public:
            UniformBufferManager(std::string name, UBOBindingPoints point, size_t size) : binding_point(point) {
                this->uniform_block_name = name;
                this->elements.resize(size);
                this->size_of_total_block = elements.size() * sizeof(T);
                bindUniformBlockBuffer();
            }

            void registerShader(Shader &shader) {
                unsigned int unifrom_block_index = glGetUniformBlockIndex(shader.ID, this->uniform_block_name.c_str());
                glUniformBlockBinding(shader.ID, unifrom_block_index, binding_point);
            }

            UniformBufferManager<T>& setElement(size_t index, T* element) {
                T* start_adress = elements.data() + index;
                glBindBuffer(GL_UNIFORM_BUFFER, (this->uniform_buffer));
                glBufferSubData(GL_UNIFORM_BUFFER, start_adress, sizeof(T), element);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
                elements[index] = *element;
                return *this;
            }

            T getElement(size_t index) {
                return elements[index];
            }

            ~UniformBufferManager() {
                glDeleteBuffers(1, &(this->uniform_buffer));
            }


        private:

            void bindUniformBlockBuffer() {
                glGenBuffers(1, &(this->uniform_buffer));
                glBindBuffer(GL_UNIFORM_BUFFER, this->uniform_buffer);
                glBufferData(GL_UNIFORM_BUFFER, size_of_total_block, nullptr, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, (this->uniform_buffer));
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
    };

    class CameraMatricesManager {
        public:
            CameraMatricesManager(std::string name, UBOBindingPoints point = UBOPOINT_CAMERA_MATRICES) : binding_point(point) {
                this->uniform_block_name = name;
                bindUniformBlockBuffer();
            }

            ~CameraMatricesManager() {
                glDeleteBuffers(1, &(this->uniform_buffer));
            }

            void registerShader(Shader &shader) {
                unsigned int unifrom_block_index = glGetUniformBlockIndex(shader.ID, this->uniform_block_name.c_str());
                glUniformBlockBinding(shader.ID, unifrom_block_index, binding_point);
            }

            void updateView(glm::mat4 &viewMat) {
                glBindBuffer(GL_UNIFORM_BUFFER, (this->uniform_buffer));
                glBufferSubData(GL_UNIFORM_BUFFER, view_start_pos, view_size, glm::value_ptr(viewMat));
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }

            void updateProjection(glm::mat4 &projectMat) {
                glBindBuffer(GL_UNIFORM_BUFFER, (this->uniform_buffer));
                glBufferSubData(GL_UNIFORM_BUFFER, projection_start_pos, projection_size, glm::value_ptr(projectMat));
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
        private:
            std::string uniform_block_name;
            unsigned int uniform_buffer;
            UBOBindingPoints binding_point;

            const GLsizeiptr view_start_pos = 0;
            const GLsizeiptr view_size = sizeof(glm::mat4);
            const GLsizeiptr view_next_bias = 0;
            const GLsizeiptr projection_start_pos = view_start_pos + view_size + view_next_bias;
            const GLsizeiptr projection_size = sizeof(glm::mat4);
            const GLsizeiptr projection_next_bias = 0;

            const GLsizeiptr size_of_total_block = view_size + view_next_bias + projection_size + projection_next_bias;

            void bindUniformBlockBuffer() {
                glGenBuffers(1, &(this->uniform_buffer));
                glBindBuffer(GL_UNIFORM_BUFFER, this->uniform_buffer);
                std::vector<unsigned char> empty(size_of_total_block, 0);
                glBufferData(GL_UNIFORM_BUFFER, size_of_total_block, empty.data(), GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, (this->uniform_buffer));
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
    };


    class GammaManager {
        public:
            GammaManager(std::string name, UBOBindingPoints point = UBOPOINT_GAMMA_CORRECTION) : binding_point(point) {
                this->uniform_block_name = name;
                this->gamma = 1.0f;
                bindUniformBlockBuffer();
            }

            ~GammaManager() {
                glDeleteBuffers(1, &(this->uniform_buffer));
            }

            void registerShader(Shader &shader) {
                unsigned int unifrom_block_index = glGetUniformBlockIndex(shader.ID, this->uniform_block_name.c_str());
                glUniformBlockBinding(shader.ID, unifrom_block_index, binding_point);
            }

            void updateGamma(const float gamma) {
                this->gamma = gamma;
                glBindBuffer(GL_UNIFORM_BUFFER, (this->uniform_buffer));
                glBufferSubData(GL_UNIFORM_BUFFER, gamma_start_pos, gamma_size, &gamma);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }

            float getGamma() {
                return this->gamma;
            }

        private:
            std::string uniform_block_name;
            unsigned int uniform_buffer;
            UBOBindingPoints binding_point;
            float gamma;

            const GLsizeiptr gamma_start_pos = 0;
            const GLsizeiptr gamma_size = sizeof(float);
            const GLsizeiptr gamma_next_bias = 0;

            const GLsizeiptr size_of_total_block = gamma_start_pos + gamma_size + gamma_next_bias;

            void bindUniformBlockBuffer() {
                glGenBuffers(1, &(this->uniform_buffer));
                glBindBuffer(GL_UNIFORM_BUFFER, this->uniform_buffer);
                std::vector<unsigned char> empty(size_of_total_block, 0);
                glBufferData(GL_UNIFORM_BUFFER, size_of_total_block, empty.data(), GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, (this->uniform_buffer));
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
    };


    struct BlinnPhongLight_direct {
        alignas(4) GLboolean activate = GL_FALSE;

        alignas(16) glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);

        alignas(16) glm::vec3 ambient = glm::vec3(0.01f);
        alignas(16) glm::vec3 diffuse = glm::vec3(0.8f);
        alignas(16) glm::vec3 specular = glm::vec3(10.0f);

        BlinnPhongLight_direct() = default;

        BlinnPhongLight_direct(
            GLboolean act,
            const glm::vec3& dir,
            const glm::vec3& amb, const glm::vec3& diff, const glm::vec3& spec)
            : direction(dir), ambient(amb), diffuse(diff), specular(spec), activate(act) {}

        bool operator==(const BlinnPhongLight_direct& other) const {
            return this->direction == other.direction
                && this->ambient == other.ambient
                && this->diffuse == other.diffuse
                && this->specular == other.specular
                && this->activate == other.activate;
        }
    };

    struct BlinnPhongLight_point {
        alignas(4) GLboolean activate = GL_FALSE;

        alignas(16) glm::vec3 position = glm::vec3(0.0f, 1.0f, 0.0f);

        alignas(4) GLfloat constant = 0.0f;
        alignas(4) GLfloat linear = 0.0f;
        alignas(4) GLfloat quadratic = 1.0f;

        alignas(16) glm::vec3 ambient = glm::vec3(0.01f);
        alignas(16) glm::vec3 diffuse = glm::vec3(0.8f);
        alignas(16) glm::vec3 specular = glm::vec3(10.0f);


        BlinnPhongLight_point() = default;

        BlinnPhongLight_point(
            GLboolean act,
            const glm::vec3& pos, GLfloat constVal, GLfloat lin, GLfloat quad,
            const glm::vec3& amb, const glm::vec3& diff, const glm::vec3& spec)
            : activate(act), position(pos), constant(constVal), linear(lin), quadratic(quad),
            ambient(amb), diffuse(diff), specular(spec) {}

        bool operator==(const BlinnPhongLight_point& other) const {
            return this->position == other.position
                && this->constant == other.constant
                && this->linear == other.linear
                && this->quadratic == other.quadratic
                && this->ambient == other.ambient
                && this->diffuse == other.diffuse
                && this->specular == other.specular
                && this->activate == other.activate;
        }
    };

    struct BlinnPhongLight_spot {
        alignas(4) GLboolean activate = GL_FALSE;

        alignas(16) glm::vec3 position = glm::vec3(0.0f, 1.0f, 0.0f);
        alignas(16) glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);

        alignas(4) GLfloat innerCutOff = 40.0f;
        alignas(4) GLfloat outerCutOff = 45.0f;

        alignas(4) GLfloat constant = 0.0f;
        alignas(4) GLfloat linear = 0.0f;
        alignas(4) GLfloat quadratic = 1.0f;

        alignas(16) glm::vec3 ambient = glm::vec3(0.01f);
        alignas(16) glm::vec3 diffuse = glm::vec3(0.8f);
        alignas(16) glm::vec3 specular = glm::vec3(10.0f);


        BlinnPhongLight_spot() = default;

        BlinnPhongLight_spot(
            GLboolean act,
            const glm::vec3& pos, const glm::vec3& dir,
            GLfloat innerCO, GLfloat outerCO, GLfloat constVal, GLfloat lin, GLfloat quad,
            const glm::vec3& amb, const glm::vec3& diff, const glm::vec3& spec)
            : activate(act), position(pos), direction(dir), innerCutOff(innerCO), outerCutOff(outerCO),
            constant(constVal), linear(lin), quadratic(quad),
            ambient(amb), diffuse(diff), specular(spec) {}

        bool operator==(const BlinnPhongLight_spot& other) const {
            return this->position == other.position
                && this->direction == other.direction
                && this->innerCutOff == other.innerCutOff
                && this->outerCutOff == other.outerCutOff
                && this->constant == other.constant
                && this->linear == other.linear
                && this->quadratic == other.quadratic
                && this->ambient == other.ambient
                && this->diffuse == other.diffuse
                && this->specular == other.specular
                && this->activate == other.activate;
        }
    };


    
    class GlobalBlinnPongLightManager {
        public:
            GlobalBlinnPongLightManager(std::string uniform_block_name, UBOBindingPoints point, const size_t dir_num, const size_t point_num, const size_t spot_num) : binding_point(point), max_of_dirLight(dir_num), max_of_pointLight(point_num), max_of_spotLight(spot_num) {
                // Set up unifrom block name
                this->uniform_block_name = uniform_block_name;
                dirLights = new BlinnPhongLight_direct[max_of_dirLight]();
                pointLights = new BlinnPhongLight_point[max_of_pointLight]();
                spotLights = new BlinnPhongLight_spot[max_of_spotLight]();
                // Initialize the unifrom block buffer
                bindUniformBlockBuffer();
                /*
                std::cout << "Dir size:" << sizeof(BlinnPhongLight_direct) << std::endl;
                std::cout << "point size:" << sizeof(BlinnPhongLight_point) << std::endl;
                std::cout << "spot size:" << sizeof(BlinnPhongLight_spot) << std::endl;
                */
            }

            // Delete all the sources which use dynamic memory and VRAM
            ~GlobalBlinnPongLightManager() {
                glDeleteBuffers(1, &unifrom_buffer);
                delete[] dirLights;
                delete[] pointLights;
                delete[] spotLights;
            }

            // Bind the given shader onto the light uniform block points
            void registerShader(Shader& shader) {
                unsigned int unifrom_block_index = glGetUniformBlockIndex(shader.ID, uniform_block_name.c_str());
                glUniformBlockBinding(shader.ID, unifrom_block_index, this->binding_point);
            }

            // edit three type of lights : directional light
            void updateDirLight(BlinnPhongLight_direct light_data, const size_t index) {
                if (index < max_of_dirLight) {
                    dirLights[index] = light_data;
                    glBindBuffer(GL_UNIFORM_BUFFER, this->unifrom_buffer);

                    glBufferSubData(GL_UNIFORM_BUFFER,
                        static_cast<GLintptr>((&dirLights[index] - dirLights) * sizeof(BlinnPhongLight_direct)),
                        static_cast<GLsizeiptr>(sizeof(BlinnPhongLight_direct)),
                        &light_data);

                    glBindBuffer(GL_UNIFORM_BUFFER, 0);
                    // std::cout << static_cast<GLintptr>((&dirLights[index] - dirLights) * sizeof(BlinnPhongLight_direct)) << std::endl;
                }
                else {
                    std::cerr << "ERROR::CUSTOMHELPER::LIGHTMANAGER::excess the max number of dirLight. (Index: " << index << ")\n";
                }
                
            }

            // edit three type of lights : point light
            void updatePointLight(BlinnPhongLight_point light_data, const size_t index) {
                if (index < max_of_pointLight) {
                    pointLights[index] = light_data;
                    glBindBuffer(GL_UNIFORM_BUFFER, this->unifrom_buffer);

                    glBufferSubData(GL_UNIFORM_BUFFER,
                                    static_cast<GLintptr>(max_of_dirLight * sizeof(BlinnPhongLight_direct) + (&pointLights[index] - pointLights) * sizeof(BlinnPhongLight_point)),
                                    static_cast < GLsizeiptr>(sizeof(BlinnPhongLight_point)),
                                    &light_data);

                    glBindBuffer(GL_UNIFORM_BUFFER, 0);
                    // std::cout << static_cast<GLintptr>(max_of_dirLight * sizeof(BlinnPhongLight_direct) + (&pointLights[index] - pointLights) * sizeof(BlinnPhongLight_point)) << std::endl;
                }
                else {
                    std::cerr << "ERROR::CUSTOMHELPER::LIGHTMANAGER::excess the max number of pointLight. (Index: " << index << ")\n";
                }
            }

            // edit three type of lights : spot light
            void updateSpotLight(BlinnPhongLight_spot light_data, const size_t index) {
                if (index < max_of_spotLight) {
                    spotLights[index] = light_data;
                    light_data.innerCutOff = glm::cos(glm::radians(light_data.innerCutOff));
                    light_data.outerCutOff = glm::cos(glm::radians(light_data.outerCutOff));
                    glBindBuffer(GL_UNIFORM_BUFFER, this->unifrom_buffer);
                    
                    glBufferSubData(GL_UNIFORM_BUFFER,
                                    static_cast<GLintptr>(max_of_dirLight * sizeof(BlinnPhongLight_direct) + max_of_pointLight * sizeof(BlinnPhongLight_point) +
                                    (&spotLights[index] - spotLights) * sizeof(BlinnPhongLight_spot)),
                                    static_cast<GLsizeiptr>(sizeof(BlinnPhongLight_spot)),
                                    &light_data);

                    glBindBuffer(GL_UNIFORM_BUFFER, 0);
                    // std::cout << static_cast<GLintptr>(max_of_dirLight * sizeof(BlinnPhongLight_direct) + max_of_pointLight * sizeof(BlinnPhongLight_point) + (&spotLights[index] - spotLights) * sizeof(BlinnPhongLight_spot)) << std::endl;
                }
                else {
                    std::cerr << "ERROR::CUSTOMHELPER::LIGHTMANAGER::excess the max number of spotLight. (Index: " << index << ")\n";
                }
            }

            // Return the index light data
            BlinnPhongLight_direct getDirLight(const size_t index) const {
                BlinnPhongLight_direct return_light = {};
                if (index < max_of_dirLight)
                    return_light = dirLights[index];
                return return_light;
            }
            BlinnPhongLight_point getpointLight(const size_t index) const {
                BlinnPhongLight_point return_light = {};
                if (index < max_of_pointLight)
                    return_light = pointLights[index];
                return return_light;
            }
            BlinnPhongLight_spot getspotLight(const size_t index) const {
                BlinnPhongLight_spot return_light = {};
                if (index < max_of_spotLight)
                    return_light = spotLights[index];
                return return_light;
            }

        private:
            // Buffer Index
            unsigned int unifrom_buffer;
            // UBO binding point
            UBOBindingPoints binding_point;
            // Uniform Block Name
            std::string uniform_block_name;
            // lights data records
            BlinnPhongLight_direct *dirLights;
            BlinnPhongLight_point *pointLights;
            BlinnPhongLight_spot *spotLights;

            // Record the maximum number of each light
            size_t max_of_dirLight;
            size_t max_of_pointLight;
            size_t max_of_spotLight;


            // Unifrom block size constant
            // uniform total size
            const GLsizeiptr size_of_total_light = (max_of_dirLight   * sizeof(BlinnPhongLight_direct)) +
                                                   (max_of_pointLight * sizeof(BlinnPhongLight_point )) +
                                                   (max_of_spotLight  * sizeof(BlinnPhongLight_spot  ));


            // Create a buffer to sotred lights data
            void bindUniformBlockBuffer() {
                glGenBuffers(1, &(this->unifrom_buffer));
                glBindBuffer(GL_UNIFORM_BUFFER, (this->unifrom_buffer));
                // Bind to GL_DYNAMIC_DRAW is really important that if don't do so, here will cause some buffer copy problems
                std::vector<unsigned char> empty(size_of_total_light, 0);
                glBufferData(GL_UNIFORM_BUFFER, size_of_total_light, empty.data(), GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_UNIFORM_BUFFER, this->binding_point, (this->unifrom_buffer));
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
    };


    class ShadowMatricesManager {
        public:
            ShadowMatricesManager(std::string name, UBOBindingPoints point, const size_t dir_num, const size_t spot_num) : binding_point(point), max_of_dirLight(dir_num), max_of_spotLight(spot_num) {
                this->uniform_block_name = name;
                this->dirMats = new glm::mat4[dir_num];
                this->spotMats = new glm::mat4[spot_num];
                bindUniformBlockBuffer();
            }

            ~ShadowMatricesManager() {
                glDeleteBuffers(1, &(this->uniform_buffer));
            }

            void registerShader(Shader &shader) {
                unsigned int unifrom_block_index = glGetUniformBlockIndex(shader.ID, this->uniform_block_name.c_str());
                glUniformBlockBinding(shader.ID, unifrom_block_index, binding_point);
            }

            void updateDirLightMat(glm::mat4 &matrix, const size_t index) {
                if (index < max_of_dirLight) {
                    this->dirMats[index] = matrix;
                    glBindBuffer(GL_UNIFORM_BUFFER, (this->uniform_buffer));
                    glBufferSubData(GL_UNIFORM_BUFFER, dirMat_start_pos + index * dirMat_size, dirMat_size, &matrix);
                    glBindBuffer(GL_UNIFORM_BUFFER, 0);
                }
                else {
                    std::cerr << "ERROR::CUSTOMHELPER::SHADOWMATRICES::excess the max number of dirLight. (Index: " << index << ")\n";
                }
            }

            void updateSpotLightMat(glm::mat4 &matrix, const size_t index) {
                if (index < max_of_spotLight) {
                    this->spotMats[index] = matrix;
                    glBindBuffer(GL_UNIFORM_BUFFER, (this->uniform_buffer));
                    glBufferSubData(GL_UNIFORM_BUFFER, spotMat_start_pos + index * spotMat_size, spotMat_size, &matrix);
                    glBindBuffer(GL_UNIFORM_BUFFER, 0);
                }
                else {
                    std::cerr << "ERROR::CUSTOMHELPER::SHADOWMATRICES::excess the max number of spotLight. (Index: " << index << ")\n";
                }
            }

            glm::mat4 getDirMat(const size_t index) const {
                return this->dirMats[index];
            }

            glm::mat4 getSpotMat(const size_t index) const {
                return this->spotMats[index];
            }

        private:
            std::string uniform_block_name;
            unsigned int uniform_buffer;
            UBOBindingPoints binding_point;
            glm::mat4 *dirMats;
            glm::mat4 *spotMats;

            const size_t max_of_dirLight;
            const size_t max_of_spotLight;

            const GLsizeiptr dirMat_start_pos = 0;
            const GLsizeiptr dirMat_size = sizeof(glm::mat4);
            const GLsizeiptr dirMat_total_size = max_of_dirLight * dirMat_size;
            const GLsizeiptr dirMat_next_arr_bias = 0;

            const GLsizeiptr spotMat_start_pos = dirMat_start_pos + dirMat_total_size + dirMat_next_arr_bias;
            const GLsizeiptr spotMat_size = sizeof(glm::mat4);
            const GLsizeiptr spotMat_total_size = max_of_spotLight * spotMat_size;
            const GLsizeiptr spotMat_next_arr_bias = 0;

            const GLsizeiptr size_of_total_block = dirMat_total_size + dirMat_next_arr_bias + spotMat_total_size + spotMat_next_arr_bias;

            void bindUniformBlockBuffer() {
                glGenBuffers(1, &(this->uniform_buffer));
                glBindBuffer(GL_UNIFORM_BUFFER, this->uniform_buffer);
                std::vector<unsigned char> empty(size_of_total_block);
                std::fill(empty.begin(), empty.end(), 0);
                glBufferData(GL_UNIFORM_BUFFER, size_of_total_block, empty.data(), GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, (this->uniform_buffer));
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
    };


    class ShadowFarPlaneManager {
    public:
        ShadowFarPlaneManager(std::string name, UBOBindingPoints point, const size_t point_num) : binding_point(point), max_of_pointLight(point_num) {
            this->uniform_block_name = name;
            this->far_planes = new float[point_num];
            bindUniformBlockBuffer();
        }

        ~ShadowFarPlaneManager() {
            delete[] far_planes;
            glDeleteBuffers(1, &(this->uniform_buffer));
        }

        void registerShader(Shader &shader) {
            unsigned int unifrom_block_index = glGetUniformBlockIndex(shader.ID, this->uniform_block_name.c_str());
            glUniformBlockBinding(shader.ID, unifrom_block_index, binding_point);
        }

        void updateFarPlane(float const &far_plane, const size_t index) {
            if (index < max_of_pointLight) {
                this->far_planes[index] = far_plane;
                glBindBuffer(GL_UNIFORM_BUFFER, (this->uniform_buffer));
                glBufferSubData(GL_UNIFORM_BUFFER, pointFar_start_pos + index * pointFar_size, pointFar_size, &far_plane);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
            else {
                std::cerr << "ERROR::CUSTOMHELPER::FARPLANEMANAGER::excess the max number of pointLight. (Index: " << index << ")\n";
            }
        }

        float getFarPlane(const size_t index) const {
            return this->far_planes[index];
        }

    private:
        std::string uniform_block_name;
        unsigned int uniform_buffer;
        UBOBindingPoints binding_point;
        float *far_planes;
        
        const size_t max_of_pointLight;

        const GLsizeiptr pointFar_start_pos = 0;
        const GLsizeiptr pointFar_size = sizeof(float) + 3 * sizeof(float);    // Actually the float in array should as big as vec4, which is 4 * sizeof(float)
        const GLsizeiptr pointFar_total_size = max_of_pointLight * pointFar_size;
        const GLsizeiptr pointFar_next_arr_bias = 0;

        const GLsizeiptr size_of_total_block = pointFar_total_size + pointFar_next_arr_bias;

        void bindUniformBlockBuffer() {
            glGenBuffers(1, &(this->uniform_buffer));
            glBindBuffer(GL_UNIFORM_BUFFER, this->uniform_buffer);
            glBufferData(GL_UNIFORM_BUFFER, size_of_total_block, nullptr, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, (this->uniform_buffer));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
    };


    inline void GenDepthFramebuffers(const unsigned int n, unsigned int *const framebuffers) {
        glGenFramebuffers(n, framebuffers);
        for (unsigned int i = 0; i < n; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    inline void GenDirDepthMaps(const unsigned int n, unsigned int* textures, const unsigned int width, const unsigned int height) {
        glGenTextures(1, textures);
        glBindTexture(GL_TEXTURE_2D_ARRAY, *textures);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, width, height, n);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        // Set the image wraping to clamp to border and set the border to 1.0 so whenever the sample outside the depth map, it always return 1.0 and the shadow value will be 0.0
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    inline void GenCubeDepthMaps(const unsigned int n, unsigned int* textures, const unsigned int width, const unsigned int height) {
        glGenTextures(1, textures);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, *textures);
        glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 1, GL_DEPTH_COMPONENT24, width, height, 6u * n);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
    }


    class GlobalBlinnPongShadowLightManager {
    public:
        //============================
        // Structures for storing previous update params
        //============================
        struct LastDirLightParam {
            BlinnPhongLight_direct light;
            float view_space;
            float far_plane;
            float line_bias;
            glm::vec3 pos_bias;

            bool operator==(const LastDirLightParam& other) const {
                return  light == other.light &&
                        view_space == other.view_space &&
                        far_plane == other.far_plane &&
                        line_bias == other.line_bias &&
                        pos_bias == other.pos_bias;
            }
        };

        struct LastPointLightParam {
            BlinnPhongLight_point light;
            float far_plane;

            bool operator==(const LastPointLightParam& other) const {
                return light == other.light &&
                    far_plane == other.far_plane;
            }
        };

        struct LastSpotLightParam {
            BlinnPhongLight_spot light;
            float angle;
            float far_plane;

            bool operator==(const LastSpotLightParam& other) const {
                return light == other.light &&
                    angle == other.angle &&
                    far_plane == other.far_plane;
            }
        };

        GlobalBlinnPongShadowLightManager(
            std::string light_ubo_name,
            std::string matrices_ubo_name,
            std::string far_plane_ubo_name,
            UBOBindingPoints light_point,
            UBOBindingPoints matrices_point,
            UBOBindingPoints far_plane_point,
            size_t dir_num,
            size_t point_num,
            size_t spot_num,
            unsigned int dir_resolution,
            unsigned int point_resolution,
            unsigned int spot_resolution
        )
        : lightManager(light_ubo_name, light_point, dir_num, point_num, spot_num)
        , matricesManager(matrices_ubo_name, matrices_point, dir_num, spot_num)
        , farPlaneManager(far_plane_ubo_name, far_plane_point, point_num)
        , num_of_dirLight(dir_num)
        , num_of_pointLight(point_num)
        , num_of_spotLight(spot_num)
        , dirMapResolution(dir_resolution)
        , pointMapResolution(point_resolution)
        , spotMapResolution(spot_resolution)
        , dir_bake_shader("shaders/bake/depth_map/vertex/dir-depth_map.vert", 
                        "shaders/bake/depth_map/fragment/dir-depth_map.frag")
        , cube_bake_shader("shaders/bake/depth_map/vertex/cube-depth_map.vert", 
                        "shaders/bake/depth_map/fragment/cube-depth_map.frag", 
                        "shaders/bake/depth_map/geometry/cube-depth_map.geom")
        {
            dir_depthMaps = 0;
            point_depthMaps = 0;
            spot_depthMaps = 0;

            // Create FBO and depth textures
            GenDepthFramebuffers(1, &this->framebuffer);
            GenDirDepthMaps(num_of_dirLight, &(this->dir_depthMaps), dirMapResolution, dirMapResolution);
            GenCubeDepthMaps(num_of_pointLight, &(this->point_depthMaps), pointMapResolution, pointMapResolution);
            GenDirDepthMaps(num_of_spotLight, &(this->spot_depthMaps), spotMapResolution, spotMapResolution);

            // Initialize previous-params vectors
            lastDirLightParams.resize(num_of_dirLight);
            lastPointLightParams.resize(num_of_pointLight);
            lastSpotLightParams.resize(num_of_spotLight);
        }

        ~GlobalBlinnPongShadowLightManager() {
            glDeleteFramebuffers(1, &this->framebuffer);
            glDeleteTextures(1, &dir_depthMaps);
            glDeleteTextures(1, &point_depthMaps);
            glDeleteTextures(1, &spot_depthMaps);
            dir_bake_shader.clear();
            cube_bake_shader.clear();
        }

        void registerShader(Shader &shader) {
            lightManager.registerShader(shader);
            matricesManager.registerShader(shader);
            farPlaneManager.registerShader(shader);
            shader.use();
            shader.setInt("shadowMaps.dirLights", SAMPLER_DIRSHADOW0);
            shader.setInt("shadowMaps.pointLights", SAMPLER_POINTSHADOW0);
            shader.setInt("shadowMaps.spotLights", SAMPLER_SPOTSHADOW0);
        }

        void changeDirShadowBakeShader(Shader &shadow_bake_shader) {
            this->dir_bake_shader = shadow_bake_shader;
        }

        void changeCubeShadowBakeShader(Shader &shadow_bake_shader) {
            this->cube_bake_shader = shadow_bake_shader;
        }

        //============================
        // Update Dir Light with param comparison
        //============================
        void updateDirLight(
            BlinnPhongLight_direct light,
            size_t index,
            float view_space,
            float far_plane,
            float line_bias,
            glm::vec3 pos_bias,
            std::function<void(Shader &)> shadow_bake_function
        ) {
            if (index >= num_of_dirLight) return;

            light.direction = glm::normalize(light.direction);

            LastDirLightParam newParam {
                light,
                view_space,
                far_plane,
                line_bias,
                pos_bias
            };

            // Skip if same as previous
            if (newParam == lastDirLightParams[index]) {
                return;
            }
            lastDirLightParams[index] = newParam;

            // Render shadow map
            glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->dir_depthMaps, 0, index);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
                glm::vec3 lightPos = light.direction * -line_bias + pos_bias;
                glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
                if (glm::all(glm::epsilonEqual(glm::abs(light.direction), glm::vec3(0.0f, 1.0f, 0.0f), 0.0001f))) {
                    up = glm::vec3(1.0f, 0.0f, 0.0f);
                }
                glm::mat4 lightProjection = glm::ortho(-view_space, view_space, -view_space, view_space, 0.1f, far_plane);
                glm::mat4 lightView = glm::lookAt(lightPos, lightPos + light.direction, up);
                glm::mat4 lightSpaceMatrix = lightProjection * lightView;

                lightManager.updateDirLight(light, index);
                matricesManager.updateDirLightMat(lightSpaceMatrix, index);

                glViewport(0, 0, this->dirMapResolution, this->dirMapResolution);
                glClear(GL_DEPTH_BUFFER_BIT);

                this->dir_bake_shader.use();
                this->dir_bake_shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
                shadow_bake_function(dir_bake_shader);

                glUseProgram(0);
            } else {
                std::cout << "ERROR::FRAMEBUFFER:: Dir shadow map incomplete!\n";
            }
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        //============================
        // Update Point Light with param comparison
        //============================
        void updatePointLight(
            BlinnPhongLight_point &light,
            size_t index,
            float far_plane,
            std::function<void(Shader &)> shadow_bake_function
        ) {
            if (index >= num_of_pointLight) return;

            LastPointLightParam newParam {
                light,
                far_plane
            };

            if (newParam == lastPointLightParams[index]) {
                return;
            }
            lastPointLightParams[index] = newParam;

            glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->point_depthMaps, 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
                glm::vec3 lightPos = light.position;
                glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f),
                    static_cast<float>(this->pointMapResolution) / static_cast<float>(this->pointMapResolution),
                    0.1f, far_plane);
                std::vector<glm::mat4> lightTransform = {
                    lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                    lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                    lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
                    lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
                    lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                    lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                };

                lightManager.updatePointLight(light, index);
                farPlaneManager.updateFarPlane(far_plane, index);

                glViewport(0, 0, this->pointMapResolution, this->pointMapResolution);

                float clear_value = 1.0f;
                glClearTexSubImage(
                    this->point_depthMaps, 
                    0, 0, 0, 
                    6 * index, // offset in the 3D layer
                    pointMapResolution, 
                    pointMapResolution, 
                    6, 
                    GL_DEPTH_COMPONENT, 
                    GL_FLOAT, 
                    &clear_value
                );

                this->cube_bake_shader.use();
                for (unsigned int i = 0; i < 6; i++) {
                    this->cube_bake_shader.setMat4("shadowMatrices[" + std::to_string(i) + "]", lightTransform[i]);
                }
                this->cube_bake_shader.setInt("layer", index);
                this->cube_bake_shader.setVec3("lightPos", lightPos);
                this->cube_bake_shader.setFloat("far_plane", far_plane);

                shadow_bake_function(this->cube_bake_shader);

                glUseProgram(0);
            } else {
                std::cout << "ERROR::FRAMEBUFFER:: Point shadow map incomplete!\n";
            }
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        //============================
        // Update Spot Light with param comparison
        //============================
        void updateSpotLight(
            BlinnPhongLight_spot &light,
            size_t index,
            float angle,
            float far_plane,
            std::function<void(Shader &)> shadow_bake_function
        ) {
            if (index >= num_of_spotLight) return;

            LastSpotLightParam newParam {
                light,
                angle,
                far_plane
            };

            if (newParam == lastSpotLightParams[index]) {
                return;
            }
            lastSpotLightParams[index] = newParam;

            glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->spot_depthMaps, 0, index);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
                glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
                if (glm::all(glm::epsilonEqual(glm::abs(light.direction), glm::vec3(0.0f, 1.0f, 0.0f), 0.0001f))) {
                    up = glm::vec3(1.0f, 0.0f, 0.0f);
                }
                glm::mat4 lightProjection = glm::perspective(glm::radians(angle),
                    static_cast<float>(this->spotMapResolution) / static_cast<float>(this->spotMapResolution),
                    0.1f, far_plane);
                glm::mat4 lightView = glm::lookAt(light.position, light.position + light.direction, up);
                glm::mat4 lightSpaceMatrix = lightProjection * lightView;

                lightManager.updateSpotLight(light, index);
                matricesManager.updateSpotLightMat(lightSpaceMatrix, index);

                glViewport(0, 0, this->spotMapResolution, this->spotMapResolution);
                glClear(GL_DEPTH_BUFFER_BIT);

                this->dir_bake_shader.use();
                this->dir_bake_shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
                shadow_bake_function(dir_bake_shader);

                glUseProgram(0);
            } else {
                std::cout << "ERROR::FRAMEBUFFER:: Spot shadow map incomplete!\n";
            }
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void bindShadowMaps() {
            glActiveTexture(GL_TEXTURE0 + SAMPLER_DIRSHADOW0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, this->dir_depthMaps);

            glActiveTexture(GL_TEXTURE0 + SAMPLER_POINTSHADOW0);
            glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, this->point_depthMaps);

            glActiveTexture(GL_TEXTURE0 + SAMPLER_SPOTSHADOW0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, this->spot_depthMaps);

            glActiveTexture(GL_TEXTURE0);
        }

        void unbindShadowMaps() {
            glActiveTexture(GL_TEXTURE0 + SAMPLER_DIRSHADOW0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

            glActiveTexture(GL_TEXTURE0 + SAMPLER_POINTSHADOW0);
            glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

            glActiveTexture(GL_TEXTURE0 + SAMPLER_SPOTSHADOW0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

            glActiveTexture(GL_TEXTURE0);
        }

        unsigned int getDirDepthMapArray(size_t /*index*/) {
            return this->dir_depthMaps;
        }

        unsigned int getPointDepthMapArray() {
            return this->point_depthMaps;
        }

        unsigned int getSpotDepthMapArray(size_t /*index*/) {
            return this->spot_depthMaps;
        }

        GlobalBlinnPongLightManager& getLightManager() {
            return lightManager;
        }

    private:
        GlobalBlinnPongLightManager lightManager;
        ShadowMatricesManager matricesManager;
        ShadowFarPlaneManager farPlaneManager;

        const size_t num_of_dirLight;
        const size_t num_of_pointLight;
        const size_t num_of_spotLight;
        unsigned int framebuffer;
        unsigned int dir_depthMaps;
        unsigned int point_depthMaps;
        unsigned int spot_depthMaps;
        const unsigned int dirMapResolution;
        const unsigned int pointMapResolution;
        const unsigned int spotMapResolution;

        Shader dir_bake_shader;
        Shader cube_bake_shader;

        // Previous params
        std::vector<LastDirLightParam>   lastDirLightParams;
        std::vector<LastPointLightParam> lastPointLightParams;
        std::vector<LastSpotLightParam>  lastSpotLightParams;
    };


    // Calculate the normal matrix
    inline glm::mat3 CalculateNormalMat(const glm::mat4 &modelMat) {
        return glm::mat3(glm::transpose(glm::inverse(modelMat)));
    }

    // Random color generator
    inline glm::vec3 GenerateRandomColor(glm::vec3 min_col = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 max_col = glm::vec3(1.0f, 1.0f, 1.0f)) {

        // Original generator version
        /*
        glm::vec3 color(0.0f);
        srand(glfwGetTime() * 1000000 + time(NULL));
        glm::vec3 range = (glm::abs(max_col - min_col) * glm::vec3(255.0f, 255.0f, 255.0f)) + glm::vec3(1.0f);
        color.r = (rand() % static_cast<int>(range.r)) / 255.0f + fminf(min_col.r, max_col.r);
        color.g = (rand() % static_cast<int>(range.g)) / 255.0f + fminf(min_col.g, max_col.g);
        color.b = (rand() % static_cast<int>(range.b)) / 255.0f + fminf(min_col.b, max_col.b);        
        */

        // Lastest optimized version, using STL random generator
        typedef std::mt19937 random_engine;
        std::random_device rd;
        random_engine ran_gen(rd());
        std::uniform_real_distribution<> real_dist_r(min_col.r, max_col.r);
        std::uniform_real_distribution<> real_dist_g(min_col.g, max_col.g);
        std::uniform_real_distribution<> real_dist_b(min_col.b, max_col.b);

        glm::vec3 color(0.0f);
        color.r = static_cast<float>(real_dist_r(ran_gen));
        color.g = static_cast<float>(real_dist_g(ran_gen));
        color.b = static_cast<float>(real_dist_b(ran_gen));

        return color;
    }

    // Random vec3 generator
    inline glm::vec3 GenerateRandomVec3(const glm::vec3 min_vec3, const glm::vec3 max_vec3) {
        
        // Original generator version
        /*
        // Set up random seed and range
        srand(glfwGetTime() * 1000000 + time(NULL));
        glm::vec3 range = (glm::abs(max_vec3 - min_vec3) * glm::vec3(100.0f, 100.0f, 100.0f)) + glm::vec3(1.0f);
        glm::vec3 ran_vec;

        // Generate random vector
        ran_vec.x = (rand() % static_cast<int>(range.x)) / 100.0f + fminf(min_vec3.x, max_vec3.x);
        ran_vec.y = (rand() % static_cast<int>(range.y)) / 100.0f + fminf(min_vec3.y, max_vec3.y);
        ran_vec.z = (rand() % static_cast<int>(range.z)) / 100.0f + fminf(min_vec3.z, max_vec3.z);        
        */

        // Lastest optimized version, using STL random generator
        // Set up distribute random number generator
        typedef std::mt19937 random_engine;
        std::random_device rd;
        random_engine ran_gen(rd());
        std::uniform_real_distribution<> real_dist_x(min_vec3.x, max_vec3.x);
        std::uniform_real_distribution<> real_dist_y(min_vec3.y, max_vec3.y);
        std::uniform_real_distribution<> real_dist_z(min_vec3.z, max_vec3.z);

        // Generate random vector
        glm::vec3 ran_vec;
        ran_vec.x = static_cast<float>(real_dist_x(ran_gen));
        ran_vec.y = static_cast<float>(real_dist_y(ran_gen));
        ran_vec.z = static_cast<float>(real_dist_z(ran_gen));


        return ran_vec;
    }

    inline float GenerateRandomAngle(const float min_angle, const float max_angle) {
        // Set up distribute random number generator
        typedef std::mt19937 random_engine;
        std::random_device rd;
        random_engine ran_gen(rd());
        std::uniform_real_distribution<> real_dist(min_angle, max_angle);

        // Generate random angle
        float angle = static_cast<float>(real_dist(ran_gen));

        return angle;
    }

    inline BlinnPhongLight_direct GenerateRandomGlobalBlinnPhongLight_dirLight(const glm::vec3 min_dir, const glm::vec3 max_dir, glm::vec3 min_col = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 max_col = glm::vec3(1.0f, 1.0f, 1.0f)) {
        BlinnPhongLight_direct light{};

        // Generate random color
        glm::vec3 color = GenerateRandomColor(min_col, max_col);
        // Generate random direction
        glm::vec3 direction = GenerateRandomVec3(min_dir, max_dir);

        light.direction = direction;
        light.ambient = color * 0.2f;
        light.diffuse = color * 0.5f;
        light.specular = color * 0.8f;

        return light;
    }

    inline BlinnPhongLight_point GenerateRandomGlobalBlinnPhongLight_pointLight(const glm::vec3 min_pos, const glm::vec3 max_pos, glm::vec3 min_col = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 max_col = glm::vec3(1.0f, 1.0f, 1.0f)) {
        BlinnPhongLight_point light{};

        // Generate random color
        glm::vec3 color = GenerateRandomColor(min_col, max_col);
        // Generate random position
        glm::vec3 position = GenerateRandomVec3(min_pos, max_pos);

        light.position = position;
        light.constant = 0.0f;
        light.linear = 0.0f;
        light.quadratic = 1.0f;
        light.ambient = color * 0.2f;
        light.diffuse = color * 0.8f;
        light.specular = color;

        return light;
    }

    inline BlinnPhongLight_spot GenerateRandomGlobalBlinnPhongLight_spotLight(
        const glm::vec3 min_dir, const glm::vec3 max_dir,
        const glm::vec3 min_pos, const glm::vec3 max_pos, 
        const float min_angle  , const float max_angle,
        glm::vec3 min_col = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 max_col = glm::vec3(1.0f, 1.0f, 1.0f)
    ) 
    {
        BlinnPhongLight_spot light{};

        // Generate color
        glm::vec3 color = GenerateRandomColor(min_col, max_col);
        // Generate random direction
        glm::vec3 direction = GenerateRandomVec3(min_dir, max_dir);
        // Generate random position
        glm::vec3 position = GenerateRandomVec3(min_pos, max_pos);
        // Generate random inner cutoff and outer cutoff. The inner cutoff is between 10 and 90 degree, and the outer cutoff is always 3 degrees larger than inner cutoff.
        float angle = GenerateRandomAngle(min_angle, max_angle);
        float innerCutoff = glm::cos(glm::radians(angle));
        float outerCutoff = glm::cos(glm::radians(angle + 3.0f));

        light.direction = direction;
        light.position = position;
        light.innerCutOff = innerCutoff;
        light.outerCutOff = outerCutoff;
        light.constant = 0.0f;
        light.linear = 0.0f;
        light.quadratic = 1.0f;
        light.ambient = color * 0.02f;
        light.diffuse = color * 0.8f;
        light.specular = color;
        
        return light;
    }

    inline BlinnPhongLight_direct GenerateRandomGlobalPbrLight_dirLight(const glm::vec3 min_dir, const glm::vec3 max_dir, glm::vec3 min_col = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 max_col = glm::vec3(1.0f, 1.0f, 1.0f)) {
        BlinnPhongLight_direct light{};

        // Generate random color
        glm::vec3 color = GenerateRandomColor(min_col, max_col);
        // Generate random direction
        glm::vec3 direction = GenerateRandomVec3(min_dir, max_dir);

        light.direction = direction;
        light.ambient = color * 0.004f;
        light.diffuse = color;
        light.specular = color;

        return light;
    }

    inline BlinnPhongLight_point GenerateRandomGlobalPbrLight_pointLight(const glm::vec3 min_pos, const glm::vec3 max_pos, glm::vec3 min_col = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 max_col = glm::vec3(1.0f, 1.0f, 1.0f)) {
        BlinnPhongLight_point light{};

        // Generate random color
        glm::vec3 color = GenerateRandomColor(min_col, max_col);
        // Generate random position
        glm::vec3 position = GenerateRandomVec3(min_pos, max_pos);

        light.position = position;
        light.constant = 0.0f;
        light.linear = 0.0f;
        light.quadratic = 1.0f;
        light.ambient = color * 0.004f;
        light.diffuse = color;
        light.specular = color;

        return light;
    }

    inline BlinnPhongLight_spot GenerateRandomGlobalPbrLight_spotLight(
        const glm::vec3 min_dir, const glm::vec3 max_dir,
        const glm::vec3 min_pos, const glm::vec3 max_pos,
        const float min_angle, const float max_angle,
        glm::vec3 min_col = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 max_col = glm::vec3(1.0f, 1.0f, 1.0f)
    )
    {
        BlinnPhongLight_spot light{};

        // Generate color
        glm::vec3 color = GenerateRandomColor(min_col, max_col);
        // Generate random direction
        glm::vec3 direction = GenerateRandomVec3(min_dir, max_dir);
        // Generate random position
        glm::vec3 position = GenerateRandomVec3(min_pos, max_pos);
        // Generate random inner cutoff and outer cutoff. The inner cutoff is between 10 and 90 degree, and the outer cutoff is always 3 degrees larger than inner cutoff.
        float angle = GenerateRandomAngle(min_angle, max_angle);
        float innerCutoff = glm::cos(glm::radians(angle));
        float outerCutoff = glm::cos(glm::radians(angle + 3.0f));

        light.direction = direction;
        light.position = position;
        light.innerCutOff = innerCutoff;
        light.outerCutOff = outerCutoff;
        light.constant = 0.0f;
        light.linear = 0.0f;
        light.quadratic = 1.0f;
        light.ambient = color * 0.002f;
        light.diffuse = color;
        light.specular = color;

        return light;
    }

    inline glm::mat4 rotateFromTo(const glm::vec3& A, const glm::vec3& B) {
        glm::vec3 from = glm::normalize(A);
        glm::vec3 to   = glm::normalize(B);

        float dotValue = glm::dot(from, to);
        dotValue = glm::clamp(dotValue, -1.0f, 1.0f);

        if (std::abs(dotValue - 1.0f) < 1e-6) {
            return glm::mat4(1.0f);
        }

        if (std::abs(dotValue + 1.0f) < 1e-6) {
            glm::vec3 tmp = glm::vec3(1.0f, 0.0f, 0.0f);
            glm::vec3 axis = glm::cross(from, tmp);

            if (glm::length(axis) < 1e-6) {
                tmp  = glm::vec3(0.0f, 1.0f, 0.0f);
                axis = glm::cross(from, tmp);
            }
            axis = glm::normalize(axis);

            return glm::rotate(glm::mat4(1.0f), glm::pi<float>(), axis);
        }

        float angle = std::acos(dotValue);
        glm::vec3 axis = glm::cross(from, to);
        axis = glm::normalize(axis);

        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, axis);
        return rotation;
    }

    // Draw the point lights in the global light uniform block
    inline void DrawGlobalPointLightCube(GlobalBlinnPongLightManager &manager,const unsigned int VAO,  Shader &shader, const float scale, const unsigned int number = MAX_NUM_POINTLIGHT) {
        shader.use();
        glBindVertexArray(VAO);
        BlinnPhongLight_point light = {};
        for (unsigned int i = 0; i < number; i++) {
            light = manager.getpointLight(i);
            // If the light isn't move, we assume it has not been set yet
            if (!light.activate)
                continue;
            glm::mat4 model(1.0f);
            model = glm::translate(model, manager.getpointLight(i).position);
            model = glm::scale(model, glm::vec3(scale));
            shader.setMat4("model", model);
            shader.setVec3("lightColor", manager.getpointLight(i).specular * 1.2f);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);
        glUseProgram(0);
    }

    // Draw the point lights in the global light uniform block
    inline void DrawGlobalPointLightSphere(GlobalBlinnPongLightManager &manager,const unsigned int sphere_VAO, const size_t index_count,  Shader &shader, const float scale, const unsigned int number = MAX_NUM_POINTLIGHT) {
        shader.use();
        glBindVertexArray(sphere_VAO);
        BlinnPhongLight_point light = {};
        for (unsigned int i = 0; i < number; i++) {
            light = manager.getpointLight(i);
            // If the light isn't move, we assume it has not been set yet
            if (!light.activate)
                continue;
            glm::mat4 model(1.0f);
            model = glm::translate(model, manager.getpointLight(i).position);
            model = glm::scale(model, glm::vec3(scale));
            shader.setMat4("model", model);
            shader.setVec3("lightColor", manager.getpointLight(i).specular * 1.2f);
            shader.setVec3("direction", glm::vec3(0.0f));

            glDrawElements(GL_TRIANGLE_STRIP, index_count, GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);
        glUseProgram(0);
    }

    // Draw the spot lights in the global light uniform block
    inline void DrawGlobalSpotLightCube(GlobalBlinnPongLightManager &manager,const unsigned int VAO,  Shader &shader, const float scale, const unsigned int number = MAX_NUM_POINTLIGHT) {
        shader.use();
        glBindVertexArray(VAO);
        BlinnPhongLight_spot light = {};
        for (unsigned int i = 0; i < number; i++) {
            light = manager.getspotLight(i);
            // If the light isn't move, we assume it has not been set yet
            if (!light.activate)
                continue;
            glm::mat4 model(1.0f);
            model = glm::translate(model, manager.getspotLight(i).position);
            model = model * rotateFromTo(glm::vec3(0.0f, 1.0f, 0.0f), light.direction);
            model = glm::scale(model, glm::vec3(scale));
            shader.setMat4("model", model);
            shader.setVec3("lightColor", manager.getspotLight(i).specular * 1.2f);
            shader.setVec3("direction", light.direction);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);
        glUseProgram(0);
    }

    // Draw the spot lights in the global light uniform block
    inline void DrawGlobalSpotLightSphere(GlobalBlinnPongLightManager &manager,const unsigned int sphere_VAO, const size_t index_count,  Shader &shader, const float scale, const unsigned int number = MAX_NUM_POINTLIGHT) {
        shader.use();
        glBindVertexArray(sphere_VAO);
        BlinnPhongLight_spot light = {};
        for (unsigned int i = 0; i < number; i++) {
            light = manager.getspotLight(i);
            // If the light isn't move, we assume it has not been set yet
            if (!light.activate)
                continue;
            glm::mat4 model(1.0f);
            model = glm::translate(model, manager.getspotLight(i).position);
            model = glm::scale(model, glm::vec3(scale));
            shader.setMat4("model", model);
            shader.setVec3("lightColor", manager.getspotLight(i).specular * 1.2f);

            glDrawElements(GL_TRIANGLE_STRIP, index_count, GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);
        glUseProgram(0);
    }

    inline void DrawTexFloor(const unsigned int &quad_VAO, Shader & shader, glm::vec3 & viewPos, const unsigned int &diffuse_tex, const unsigned int &specular_tex, const float shininess, const glm::vec3 position = glm::vec3(0.0f), const float scale = 1.0f, const int tex_repeate_times = 1) {
        shader.use();
        shader.setInt("material.diffuse", 0);
        shader.setInt("material.specular", 1);
        shader.setFloat("material.shininess", shininess);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuse_tex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specular_tex);
        glm::mat4 model(1.0f);
        glm::mat3 normalMat(1.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(scale));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        normalMat = CustomHelper::CalculateNormalMat(model);
        shader.setMat4("model", model);
        shader.setMat3("normalMat", normalMat);
        shader.setVec3("viewPos", viewPos);
        shader.setInt("repeat_times", tex_repeate_times);

        glBindVertexArray(quad_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    // Lerp function
    inline float lerp(float a, float b, float f) {
        return a + f * (b - a);
    }

}

#endif