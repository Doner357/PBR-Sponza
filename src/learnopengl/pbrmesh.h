#ifndef PBRMESH_H
#define PBRMESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader_m.h"
#include "mesh.h"

#include <iostream>
#include <string>
#include <vector>


class PbrMesh {
public:
	std::vector<Vertex>       vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture>      textures;
	float shininess;

	PbrMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, float shininess) {
		this->vertices = vertices;
		this->indices  = indices;
		this->textures = textures;
		this->shininess = shininess;

		setupMesh();
	}
	void Draw(Shader &shader) {
		unsigned int albedoNr  = 1;
		unsigned int metallicNr = 1;
        unsigned int roughnessNr = 1;
		unsigned int normalNr   = 1;
		unsigned int heightNr   = 1;
		unsigned int aoNr = 1;
		unsigned int opacityNr = 1;
		for (unsigned int i = 0; i < textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			// Retrieve texture number (the n diffuse_textureN)
			std::string number;
			std::string name = textures[i].type;
			if (name == "albedo")
				number = std::to_string(albedoNr++);
			else if (name == "metallic")
				number = std::to_string(metallicNr++);
			else if (name == "roughness")
				number = std::to_string(roughnessNr++);
			else if (name == "normal")
				number = std::to_string(normalNr++);
			else if (name == "height")
				number = std::to_string(heightNr++);
			else if (name == "ao")
				number = std::to_string(aoNr++);
			else if (name == "opacity")
				number = std::to_string(opacityNr++);

			// Now set the sampler to the correct texture unit
			if (name == "height") {
				shader.setInt("heightMap", i);
			}
			else {
				shader.setInt(("material." + name).c_str(), i);
			}
			// And finally bind the texture
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
		glActiveTexture(GL_TEXTURE0);

		//shader.setFloat("material.roughness", this->shininess);

		//Draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	unsigned int GetVAO() {
		return this->VAO;
	}

	unsigned int* GetVAO_ptr() {
		return &(this->VAO);
	}

private:
	// Render data
	unsigned int VAO, VBO, EBO;

	void setupMesh() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		// load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// A great thing about structs is that their memory layout is sequential for all its items.
		// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
		// again translates to 3/2 floats which translates to a byte array.

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// Set the vertex attribute pointers
		// Vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
		// vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
		// vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));
		// vertex tangent
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));
		// vertex bitangent
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));
			// ids
			glEnableVertexAttribArray(5);
			glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void *)offsetof(Vertex, m_BoneIDs));
			// weights
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, m_Weights));

		glBindVertexArray(0);
	}
};
#endif // !PBRMESH_H