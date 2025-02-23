#ifndef COMMON_VAO_CREATOR_H
#define COMMON_VAO_CREATOR_H

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <vector>

enum VAOenum {
	VAO_CUBE,
	VAO_COLORCUBE,
	VAO_TEXTURECUBE,
	VAO_CUBEMAP,
	VAO_SKYBOX,
	VAO_QUAD,
	VAO_COLORQUAD,
	VAO_TEXTUREQUAD
};

unsigned int CreateCommonVAO(VAOenum type, const float red, const float green, const float blue);
unsigned int CreateCommonVAO(VAOenum type, const glm::vec3 scale = glm::vec3(1.0f), const float red = 1.0f, const float green = 1.0f, const float blue = 1.0f);
unsigned int VAOsetting(VAOenum type, const glm::vec3 scale, const glm::vec3 color);

namespace vcd {
	const float cube[] = {
		// Back face
		-1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		// Front face
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		// Left face
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		// Right face
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		 // Bottom face
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		// Top face
		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
	};

	const float cubetex[] = {
		// Back face
		 0.0f, 0.0f, // Bottom-left
		 1.0f, 1.0f, // top-right
		 1.0f, 0.0f, // bottom-right         
		 1.0f, 1.0f, // top-right
		 0.0f, 0.0f, // bottom-left
		 0.0f, 1.0f, // top-left
		// Front face
		 0.0f, 0.0f, // bottom-left
		 1.0f, 0.0f, // bottom-right
		 1.0f, 1.0f, // top-right
		 1.0f, 1.0f, // top-right
		 0.0f, 1.0f, // top-left
		 0.0f, 0.0f, // bottom-left
		// Left face
		 1.0f, 0.0f, // top-right
		 1.0f, 1.0f, // top-left
		 0.0f, 1.0f, // bottom-left
		 0.0f, 1.0f, // bottom-left
		 0.0f, 0.0f, // bottom-right
		 1.0f, 0.0f, // top-right
		// Right face
		 1.0f, 0.0f, // top-left
		 0.0f, 1.0f, // bottom-right
		 1.0f, 1.0f, // top-right         
		 0.0f, 1.0f, // bottom-right
		 1.0f, 0.0f, // top-left
		 0.0f, 0.0f, // bottom-left     
		// Bottom face
		 0.0f, 1.0f, // top-right
		 1.0f, 1.0f, // top-left
		 1.0f, 0.0f, // bottom-left
		 1.0f, 0.0f, // bottom-left
		 0.0f, 0.0f, // bottom-right
		 0.0f, 1.0f, // top-right
		// Top face
		 0.0f, 1.0f, // top-left
		 1.0f, 0.0f, // bottom-right
		 1.0f, 1.0f, // top-right     
		 1.0f, 0.0f, // bottom-right
		 0.0f, 1.0f, // top-left
		 0.0f, 0.0f  // bottom-left
	};

	const float quad[] = {
		 1.0f,  1.0f,
		-1.0f,  1.0f,
		-1.0f, -1.0f,

		 1.0f, -1.0f,
		 1.0f,  1.0f,
		-1.0f, -1.0f
	};

	const float quadtex[] = {
		 1.0f,  1.0f,
		 0.0f,  1.0f,
		 0.0f,  0.0f,

		 1.0f,  0.0f,
		 1.0f,  1.0f,
		 0.0f,  0.0f
	};

	const float skybox[] = {
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
}


unsigned int CreateCommonVAO(VAOenum type, const float red, const float green, const float blue) {
	return VAOsetting(type, glm::vec3(1.0f), glm::vec3(red, green, blue));
}

unsigned int CreateCommonVAO(VAOenum type, const glm::vec3 scale, const float red, const float green, const float blue) {
	return VAOsetting(type, scale, glm::vec3(red, green, blue));
}

unsigned int VAOsetting(VAOenum type, const glm::vec3 scale, const glm::vec3 color) {
	// Store actual vertices data
	std::vector<float> vertices_data;
	// Store the index of the texture array
	unsigned int tex_index = 0;

	// Create VAO and VBO
	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Fill buffer data
	switch (type) {
		case VAO_CUBE:
			// Fill vertices data into the vector
			for (unsigned int i = 0; i < (3 * 36); i += 3) {
				// Scale each vertex with scale factor
				vertices_data.push_back(scale.x * vcd::cube[i + 0]);
				vertices_data.push_back(scale.y * vcd::cube[i + 1]);
				vertices_data.push_back(scale.z * vcd::cube[i + 2]);
			}
			glBufferData(GL_ARRAY_BUFFER, vertices_data.size() * sizeof(float), &vertices_data[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			break;
		case VAO_COLORCUBE:
			// Fill vertex position and color info into vector
			for (unsigned int i = 0; i < (3 * 36); i += 3) {
				// Scale each vertex with scale factor
				vertices_data.push_back(scale.x * vcd::cube[i + 0]);
				vertices_data.push_back(scale.y * vcd::cube[i + 1]);
				vertices_data.push_back(scale.z * vcd::cube[i + 2]);
				// Append vertex color
				vertices_data.push_back(color.r);
				vertices_data.push_back(color.g);
				vertices_data.push_back(color.b);
			}
			glBufferData(GL_ARRAY_BUFFER, vertices_data.size() * sizeof(float), &vertices_data[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			break;
		case VAO_TEXTURECUBE:
		case VAO_CUBEMAP:
			// Store the index of cube texture coordinates
			tex_index = 0;
			// Fill vertex position and texture coordinates info into vector
			for (unsigned int i = 0; i < (3 * 36); i += 3) {
				// Scale each vertex with scale factor
				vertices_data.push_back(scale.x * vcd::cube[i + 0]);
				vertices_data.push_back(scale.y * vcd::cube[i + 1]);
				vertices_data.push_back(scale.z * vcd::cube[i + 2]);
				for (unsigned int j = 0; j < 2; j++)
					vertices_data.push_back(vcd::cubetex[tex_index + j]);
				tex_index += 2;
			}
			glBufferData(GL_ARRAY_BUFFER, vertices_data.size() * sizeof(float), &vertices_data[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			break;
		case VAO_SKYBOX:
			// Fill vertices data into the vector
			for (unsigned int i = 0; i < (3 * 36); i++)
				vertices_data.push_back(vcd::skybox[i]);
			glBufferData(GL_ARRAY_BUFFER, vertices_data.size() * sizeof(float), &vertices_data[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			break;
		case VAO_QUAD:
			// Fill vertex position and texture coordinates info into vector
			for (unsigned int i = 0; i < (2 * 6); i += 2) {
				vertices_data.push_back(scale.x * vcd::quad[i + 0]);
				vertices_data.push_back(scale.y * vcd::quad[i + 1]);
			}
			glBufferData(GL_ARRAY_BUFFER, vertices_data.size() * sizeof(float), &vertices_data[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			break;
		case VAO_COLORQUAD:
			// Fill vertex position and texture coordinates info into vector
			for (unsigned int i = 0; i < (2 * 6); i += 2) {
				vertices_data.push_back(scale.x * vcd::quad[i + 0]);
				vertices_data.push_back(scale.y * vcd::quad[i + 0]);
				vertices_data.push_back(color.r);
				vertices_data.push_back(color.g);
				vertices_data.push_back(color.b);
			}
			glBufferData(GL_ARRAY_BUFFER, vertices_data.size() * sizeof(float), &vertices_data[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
		case VAO_TEXTUREQUAD:
			// Store the index of quad texture coordinates
			tex_index = 0;
			// Fill vertex position and texture coordinates info into vector
			for (unsigned int i = 0; i < (2 * 6); i += 2) {
				vertices_data.push_back(scale.x * vcd::quad[i + 0]);
				vertices_data.push_back(scale.y * vcd::quad[i + 1]);
				for (unsigned int j = 0; j < 2; j++)
					vertices_data.push_back(vcd::quadtex[tex_index + j]);
				tex_index += 2;
			}
			glBufferData(GL_ARRAY_BUFFER, vertices_data.size() * sizeof(float), &vertices_data[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
			break;
		default:
			break;
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO);

	return VAO;
}

#endif