#pragma once

// STD
#include <vector>
#include <memory>

#include "Helper.h"

class Renderer;

class Mesh {

	struct Vertex {
		float pos[3];
		float color[3];
	};

public:
	Mesh();
	~Mesh();

	void create(std::shared_ptr<Renderer> renderer);

private:

	std::vector<Vertex>   vertices;
	std::vector<uint32_t> indices;

	VkBuffer vertex_buffer;
	VkBuffer index_buffer;
};