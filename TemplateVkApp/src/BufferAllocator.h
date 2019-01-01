#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <assert.h>

#include "Buffer.h"
#include "helper.h"

class BufferAllocator
{
public:
	BufferAllocator(VkPhysicalDevice physicalDevice, VkDevice device);
	~BufferAllocator();

	Buffer createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, std::vector<uint32_t> queueFamilyIndices);
	void   freeBuffer(Buffer buffer);

	void*  mapBuffer(Buffer buffer)   const;
	void   unmapBuffer(Buffer buffer) const;

private:
	const uint32_t     BuffersInFlightFrames = 2;
	const VkDeviceSize LargeHeapBlockSize    = 256 * 1024 * 1024; // 256 MB

	static void vmaAllocateDeviceMemory(
		VmaAllocator      allocator,
		uint32_t          memoryType,
		VkDeviceMemory    memory,
		VkDeviceSize      size)
	{

	}
	

	static void vmaFreeDeviceMemory(
		VmaAllocator      allocator,
		uint32_t          memoryType,
		VkDeviceMemory    memory,
		VkDeviceSize      size)
	{

	}


	void createBuffer(
		VkBufferCreateInfo bufferInfo, 
		VmaAllocationCreateInfo allocInfo, 
		VmaAllocation& allocation, VkBuffer& buffer)
	{
		vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
	}

private:
	VmaAllocator m_allocator;
};