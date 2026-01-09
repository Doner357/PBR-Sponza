#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "shader_m.h"
#include "ddsloader.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <filesystem>

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gammaCorrection);

inline bool hasExtension(const std::string& fileName, const std::string& extension) {
	return std::filesystem::path(fileName).extension() == "." + extension;
}

class Model {
public:
	// Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
	std::vector<Texture> textures_loaded;
	// Model data
	std::vector<Mesh> meshes;
	std::string directory;
	bool gammaCorrection;
	// Constructor, expects a filepath to a 3D model.
	Model(std::string const &path, bool gamma, bool flipUVs = false) : gammaCorrection(gamma) {
		loadModel(path, flipUVs);
	}

	~Model() {
		for (unsigned int i = 0; i < this->meshes.size(); i++)
			glDeleteVertexArrays(1, this->meshes[i].GetVAO_ptr());
	}

	// Draws the model, and thus all its meshes
	void Draw(Shader &shader) {
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shader);
	}

private:

	void loadModel(std::string path, bool flipUVs) {
		unsigned int flipUVmask = 0;
		if (flipUVs)
			flipUVmask = aiPostProcessSteps::aiProcess_FlipUVs;
		// Load model into the assimp scene
		Assimp::Importer importer;
		const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | flipUVmask);
		
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));
		processNode(scene->mRootNode, scene);
	}

	void processNode(aiNode *node, const aiScene *scene) {
		// Process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// Then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
		std::vector<Vertex> verticies;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		// Load vertex attributes
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;
			glm::vec3 vector;                    // temporary loader
			// Load vertex position
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			// Load normal
			if (mesh->HasNormals()) {
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}
			else {
				vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);
			}

			// Load texture coords
			if (mesh->mTextureCoords[0]) {
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else {
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}

			if (mesh->HasTangentsAndBitangents()) {
				// tangent
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;

				// bitangent
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = vector;
			}
			else {
				vertex.Tangent = glm::vec3(0.0f);
				vertex.Bitangent = glm::vec3(0.0f);
			}

			verticies.push_back(vertex);
		}

		// Loading the draw indices of a mesh
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		// Load textures
		if (mesh->mMaterialIndex >= 0) {
			aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
			// Diffuse Maps
			std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			// Specular Maps
			std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			// 3. normal maps
			std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
			textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
			// 4. height maps
			std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_height");
			textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
			// 5. emission maps
			std::vector<Texture> emissionMaps = loadMaterialTextures(material, aiTextureType_EMISSION_COLOR, "texture_emission");
			textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
		}
		//aiGetMaterialFloat(scene->mMaterials[mesh->mMaterialIndex], AI_MATKEY_SHININESS, &shininess
		float shininess;
		if (AI_SUCCESS != scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_SHININESS, shininess))
			shininess = 16.0f;

		return Mesh(verticies, indices, textures, shininess);
	}


	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName) {
		std::vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;
			mat->GetTexture(type, i, &str);
			// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++) {
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip) {
				Texture texture;
				float gammaCorrect = (typeName == "texture_diffuse" || typeName == "albedo") && this->gammaCorrection;
				if (hasExtension(str.C_Str(), "dds")) {
					texture.id = texture_loadDDS(str.C_Str(), this->directory, gammaCorrect);
				}
				else {
					texture.id = TextureFromFile(str.C_Str(), this->directory, gammaCorrect);
				}
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
			}
		}
		return textures;
	}
};

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gammaCorrection) {
	std::string filename = std::string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data) {
		GLenum internalFormat;
		GLenum dataFormat;
		if (nrComponents == 1) {
			internalFormat = dataFormat = GL_RED;
		}
		else if (nrComponents == 3) {
			internalFormat = (gammaCorrection ? GL_SRGB : GL_RGB);
			dataFormat = GL_RGB;
		}
		else if (nrComponents == 4) {
			internalFormat = (gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA);
			dataFormat = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		GLfloat value, max_anisotropy = 8.0f; /* don't exceed this value...*/
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, & value);
		value = (value > max_anisotropy) ? max_anisotropy : value;
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);

		stbi_image_free(data);
	}
	else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
#endif // !MODEL_H
