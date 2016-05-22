#include "Mesh.h"

#include "Helper.h"
#include "Renderer.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::create(std::shared_ptr<Renderer> renderer)
{
	auto device = renderer->getVulkanDevice();

	uint32_t vertex_buffer_size = vertices.size() * sizeof(Vertex);
	uint32_t indices_buffer_size = indices.size() * sizeof(uint32_t);

	VkMemoryAllocateInfo memAllocInfo{};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memRequirements{};

	// Not a fast but a good starting method.
	VkBufferCreateInfo vertex_buffer_info{};
	vertex_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertex_buffer_info.size = vertex_buffer_size;
	vertex_buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	// Copy the vertex data to the buffer.
	CHECK_ERROR(vkCreateBuffer(device, &vertex_buffer_info, nullptr, &vertex_buffer));
	vkGetBufferMemoryRequirements(device, vertex_buffer, &memRequirements);
	memAllocInfo.allocationSize = memRequirements.size;
	//memAllocInfo.memoryTypeIndex = 
}
