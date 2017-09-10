#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>

class MeshLoader
{
public:
	MeshLoader(const std::string& meshFile);
	~MeshLoader() = default;

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
	};

public:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};