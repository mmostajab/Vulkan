#pragma once

#include <vector>

#include <glm/glm.hpp>

class Mesh
{
public:
	Mesh(const VkDeleter<VkDevice>& device);
	~Mesh();

	virtual std::vector<VkVertexInputBindingDescription> getBindingDescription() = 0;
	virtual std::vector<VkVertexInputAttributeDescription> getAttribDescription() = 0;

	virtual const void* getVertices() const = 0;
	virtual const void* getIndices() const = 0;

	virtual size_t getVertexCount() = 0;
	virtual size_t getVertexStride() = 0;
	virtual size_t getIndexCount() = 0;
	virtual size_t getIndexStide() = 0;

	VkDeleter<VkDeviceMemory> vertexBufferMem;
	VkDeleter<VkBuffer> vertexBufferId;

	VkDeleter<VkDeviceMemory> indexBufferMem;
	VkDeleter<VkBuffer> indexBufferId;
	
	VkDeleter<VkImage> textureImg;
	VkDeleter<VkImageView> textureImgView;
	VkDeleter<VkDeviceMemory> textureImgMem;
};

class ScreenSpaceMesh : public Mesh
{
public:
	ScreenSpaceMesh(const VkDeleter<VkDevice>& device);
	~ScreenSpaceMesh();

	std::vector<VkVertexInputBindingDescription> getBindingDescription() override;
	std::vector<VkVertexInputAttributeDescription> getAttribDescription() override;

	const void* getVertices() const override;
	const void* getIndices() const override;

	size_t getVertexCount() override;
	size_t getVertexStride() override;

	size_t getIndexCount() override;
	size_t getIndexStide() override;

	struct Vertex
	{
		glm::vec2 pos;
		glm::vec3 color;
	};

private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

class Mesh3D : public Mesh
{
public:
	Mesh3D(const VkDeleter<VkDevice>& device);
	~Mesh3D();

	std::vector<VkVertexInputBindingDescription> getBindingDescription() override;
	std::vector<VkVertexInputAttributeDescription> getAttribDescription() override;

	const void* getVertices() const override;
	const void* getIndices() const override;

	size_t getVertexCount() override;
	size_t getVertexStride() override;

	size_t getIndexCount() override;
	size_t getIndexStide() override;

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		bool operator==(const Mesh3D::Vertex& rhs) const
		{
			return pos == rhs.pos && color == rhs.color && texCoord == rhs.texCoord;
		}
	};

	void setVertices(const std::vector<Vertex>& _vertices);
	void setIndices(const std::vector<uint32_t>& _indices);

private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};