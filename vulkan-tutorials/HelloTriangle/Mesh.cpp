#include "stdafx.h"
#include "Mesh.h"

// ===============================================================
// == Mesh Parent Class
// ===============================================================
Mesh::Mesh(const VkDeleter<VkDevice>& device):
	vertexBufferId{ device, vkDestroyBuffer },
	vertexBufferMem{ device, vkFreeMemory },
	indexBufferId{ device, vkDestroyBuffer },
	indexBufferMem{ device, vkFreeMemory },
	textureImg{device, vkDestroyImage},
	textureImgView{device, vkDestroyImageView},
	textureImgMem{device, vkFreeMemory}
{
}


Mesh::~Mesh()
{
}

// ===============================================================
// == Screen Space Mesh Class
// ===============================================================

ScreenSpaceMesh::ScreenSpaceMesh(const VkDeleter<VkDevice>& device): Mesh(device)
{
	vertices = {
		{ {  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
		{ {  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } },
		{ { -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } },
		{ { -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } },
	};

	indices = {
		0, 1, 2,
		2, 3, 0
	};
}

ScreenSpaceMesh::~ScreenSpaceMesh()
{
}

std::vector<VkVertexInputBindingDescription> ScreenSpaceMesh::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDesc{};
	bindingDesc.binding = 0;
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	bindingDesc.stride = sizeof(Vertex);
	return { bindingDesc };
}

std::vector<VkVertexInputAttributeDescription> ScreenSpaceMesh::getAttribDescription()
{
	VkVertexInputAttributeDescription posDesc{}, colorDesc{};

	posDesc.binding = 0;
	posDesc.location = 0;
	posDesc.offset = offsetof(Vertex, pos);
	posDesc.format = VK_FORMAT_R32G32_SFLOAT;

	colorDesc.binding = 0;
	colorDesc.location = 1;
	colorDesc.offset = offsetof(Vertex, color);
	colorDesc.format = VK_FORMAT_R32G32B32_SFLOAT;

	return { posDesc, colorDesc };
}

const void * ScreenSpaceMesh::getVertices() const
{
	return vertices.data();
}

const void * ScreenSpaceMesh::getIndices() const
{
	return indices.data();
}

size_t ScreenSpaceMesh::getVertexCount()
{
	return vertices.size();
}

size_t ScreenSpaceMesh::getVertexStride()
{
	return sizeof(Vertex);
}

size_t ScreenSpaceMesh::getIndexCount()
{
	return indices.size();
}

size_t ScreenSpaceMesh::getIndexStide()
{
	return sizeof(uint32_t);
}

// ===============================================================
// == 3D Mesh Class
// ===============================================================

Mesh3D::Mesh3D(const VkDeleter<VkDevice>& device) : Mesh(device)
{
	vertices = {
		{ {  0.5f, 0.0f, -0.5f, }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
		{ { 0.5f,  0.0f,  0.5f, }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
		{ { -0.5f, 0.0f,  0.5f, }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { -0.5f, 0.0f, -0.5f, }, { 0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f } },

		{ {  0.5f, -0.2f, -0.5f, },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } },
		{ {  0.5f, -0.2f,  0.5f, },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } },
		{ { -0.5f, -0.2f,  0.5f, },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
		{ { -0.5f, -0.2f, -0.5f, },{ 0.5f, 0.5f, 0.5f },{ 0.0f, 0.0f } },
	};

	indices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};
}

Mesh3D::~Mesh3D()
{
}

std::vector<VkVertexInputBindingDescription> Mesh3D::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDesc{};
	bindingDesc.binding = 0;
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	bindingDesc.stride = sizeof(Vertex);
	return { bindingDesc };
}

std::vector<VkVertexInputAttributeDescription> Mesh3D::getAttribDescription()
{
	VkVertexInputAttributeDescription posDesc{}, colorDesc{}, texCoordDesc;

	posDesc.binding = 0;
	posDesc.location = 0;
	posDesc.offset = offsetof(Vertex, pos);
	posDesc.format = VK_FORMAT_R32G32B32_SFLOAT;

	colorDesc.binding = 0;
	colorDesc.location = 1;
	colorDesc.offset = offsetof(Vertex, color);
	colorDesc.format = VK_FORMAT_R32G32B32_SFLOAT;

	texCoordDesc.binding = 0;
	texCoordDesc.location = 2;
	texCoordDesc.offset = offsetof(Vertex, texCoord);
	texCoordDesc.format = VK_FORMAT_R32G32_SFLOAT;

	return { posDesc, colorDesc, texCoordDesc };
}

const void * Mesh3D::getVertices() const
{
	return vertices.data();
}

const void * Mesh3D::getIndices() const
{
	return indices.data();
}

size_t Mesh3D::getVertexCount()
{
	return vertices.size();
}

size_t Mesh3D::getVertexStride()
{
	return sizeof(Vertex);
}

size_t Mesh3D::getIndexCount()
{
	return indices.size();
}

size_t Mesh3D::getIndexStide()
{
	return sizeof(uint32_t);
}

void Mesh3D::setVertices(const std::vector<Vertex>& _vertices)
{
	vertices = _vertices;
}

void Mesh3D::setIndices(const std::vector<uint32_t>& _indices)
{
	indices = _indices;
}
