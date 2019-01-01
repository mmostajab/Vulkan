#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class Buffer
{
	friend class BufferAllocator;
public:
	Buffer();
	~Buffer();

	VkBuffer getVkBuffer() const;


private:
	VkBuffer             m_vkBuffer;
	VkBufferUsageFlags   m_vkBufferUsage;
	VmaMemoryUsage       m_vmaBufferUsage;
	VmaAllocation        m_vmaAllocation;
	VmaAllocationInfo    m_vmaAllocationInfo;
};