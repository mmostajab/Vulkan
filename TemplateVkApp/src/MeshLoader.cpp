#include "MeshLoader.h"

#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

MeshLoader::MeshLoader(const std::string& meshFile)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(meshFile.c_str(),
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);
	if (!scene) {
		std::cout << "[ERROR] Cannot load " << meshFile << std::endl;
		return;
	}

	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		uint32_t verticesOffset = static_cast<uint32_t>(vertices.size());

		std::vector<glm::vec3> newMeshVertices;
		std::vector<glm::vec3> newMeshIndices;

		aiMesh* mesh = scene->mMeshes[i];
		int iMeshFaces = mesh->mNumFaces;
		for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
			Vertex vertex;
			//std::cout << "Vertex #" << v << " " << mesh->mVertices[v][0] << " " << mesh->mVertices[v][1] << " " << mesh->mVertices[v][2] << std::endl;
			vertex.position = glm::vec3(mesh->mVertices[v][0], mesh->mVertices[v][1], mesh->mVertices[v][2]);
			if (mesh->mNormals)
				vertex.normal = glm::vec3(mesh->mNormals[v][0], mesh->mNormals[v][1], mesh->mNormals[v][2]);
			else
				vertex.normal = glm::vec3(0.0f);
			vertices.push_back(vertex);
		}
		for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
			aiFace& face = mesh->mFaces[f];
			for (unsigned int fi = 1; fi < face.mNumIndices - 1; fi++) {

				uint32_t vIds[3] = {
					verticesOffset + face.mIndices[0],
					verticesOffset + face.mIndices[fi + 0],
					verticesOffset + face.mIndices[fi + 1]
				};

				indices.push_back(vIds[0]);
				indices.push_back(vIds[1]);
				indices.push_back(vIds[2]);

				if (!mesh->mNormals) {

					glm::vec3 n = glm::cross(vertices[vIds[1]].position - vertices[vIds[0]].position, vertices[vIds[2]].position - vertices[vIds[0]].position);

					vertices[vIds[0]].normal += n;
					vertices[vIds[1]].normal += n;
					vertices[vIds[2]].normal += n;

				}
			}
		}
	}

	for (auto& v : vertices) {
		v.normal = glm::normalize(v.normal);
	}
}
