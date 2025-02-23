#ifndef PBRMODEL_H
#define PBRMODEL_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb/stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "pbrmesh.h"
#include "shader_m.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gammaCorrection);

class PbrModel {
public:
	// Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
	std::vector<Texture> textures_loaded;
	// Model data
	std::vector<PbrMesh> meshes;
	std::string directory;
	bool gammaCorrection;
	// Constructor, expects a filepath to a 3D model.
	PbrModel(std::string const &path, bool gamma, bool flipUVs = false) : gammaCorrection(gamma) {
		loadModel(path, flipUVs);
	}

	~PbrModel() {
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

	PbrMesh processMesh(aiMesh *mesh, const aiScene *scene) {
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
			// Albedo Maps
			std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "albedo");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			// Metalic Maps
			std::vector<Texture> metallicMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "metallic");
			textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());
			// Roughness maps
			std::vector<Texture> roughnessMaps = loadMaterialTextures(material, aiTextureType_SHININESS, "roughness");
			textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
			// normal maps
			std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "normal");
			textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
			// height maps
			std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "height");
			textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
			// height maps
			std::vector<Texture> aoMaps = loadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION, "ao");
			textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
			// height maps
			std::vector<Texture> opacityMaps = loadMaterialTextures(material, aiTextureType_OPACITY, "opacity");
			textures.insert(textures.end(), opacityMaps.begin(), opacityMaps.end());
		}
		//aiGetMaterialFloat(scene->mMaterials[mesh->mMaterialIndex], AI_MATKEY_SHININESS, &shininess
		float roughness;
		if (AI_SUCCESS != scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness))
			roughness = 1.0f;

		return PbrMesh(verticies, indices, textures, roughness);
	}

	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName) {
		std::vector<Texture> textures;
        aiString str;
        unsigned int num_of_texture = mat->GetTextureCount(type);
        std::string dir = this->directory;

        bool empty = false;
		if (num_of_texture == 0) {
            if (typeName == "albedo") {
                str = "textures/error.jpg";
                dir = ".";
            }
            else if (typeName == "metallic") {
                str = "textures/black.jpg";
                dir = ".";
            }
            else if (typeName == "roughness") {
                str = "textures/white.jpg";
                dir = ".";
            }
            else if (typeName == "normal") {
                str = "textures/flat_normal.png";
                dir = ".";
            }
            else if (typeName == "height") {
                str = "textures/white.jpg";
                dir = ".";
            }
            else if (typeName == "ao") {
                str = "textures/white.jpg";
                dir = ".";
            }
            else if (typeName == "opacity") {
                str = "textures/white.jpg";
                dir = ".";
            }

            empty = true;
            ++num_of_texture;
		}

        for (unsigned int i = 0; i < num_of_texture; i++) {
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
			if (!empty) mat->GetTexture(type, i, &str);
            //std::cout << typeName << " " << str.C_Str() << std::endl;
            for (unsigned int j = 0; j < textures_loaded.size(); j++) {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                	Texture texture;
					texture.type = typeName;
					texture.path = str.C_Str();
					texture.id = textures_loaded[j].id;
                    textures.push_back(texture);
                    skip = true;
                    break;
                }
            }
            if (!skip) {
                Texture texture;
                float gammaCorrect = (typeName == "texture_diffuse" || typeName == "albedo") && this->gammaCorrection;
                if (hasExtension(str.C_Str(), "dds")) {
                    texture.id = texture_loadDDS(str.C_Str(), dir, gammaCorrect);
                }
                else {
                    texture.id = TextureFromFile(str.C_Str(), dir, gammaCorrect);
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

#endif // !MODEL_H
