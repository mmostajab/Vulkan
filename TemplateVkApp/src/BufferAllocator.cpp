#include "BufferAllocator.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

BufferAllocator::BufferAllocator(VkPhysicalDevice physicalDevice, VkDevice device)
{
	VmaDeviceMemoryCallbacks deviceMemAllocNotificationCallback{ BufferAllocator::vmaAllocateDeviceMemory, BufferAllocator::vmaFreeDeviceMemory };


	VmaAllocatorCreateInfo createInfo{
		0,                                         // flags
		physicalDevice,
		device,
		LargeHeapBlockSize,
		nullptr,
		&deviceMemAllocNotificationCallback,
		BuffersInFlightFrames,
		nullptr,                                   // pHeapSizeLimit
		nullptr,                             
		nullptr                                    // VmaRecordSettings
	};

	VkResult result = vmaCreateAllocator(&createInfo, &m_allocator);
	assert(result == VK_SUCCESS);
}

Buffer BufferAllocator::createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, std::vector<uint32_t> queueFamilyIndices)
{
	VkBufferCreateInfo vkAllocCreateInfo{};
	vkAllocCreateInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkAllocCreateInfo.usage                 = bufferUsageFlags;
	vkAllocCreateInfo.size                  = bufferSize;
	vkAllocCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
	vkAllocCreateInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
	vkAllocCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;

	VmaMemoryUsage vmaBufferUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VmaAllocationCreateInfo vmaAllocCreateInfo{};
	vmaAllocCreateInfo.flags          = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	vmaAllocCreateInfo.usage          = vmaBufferUsage;
	vmaAllocCreateInfo.memoryTypeBits = 0;
	vmaAllocCreateInfo.pool           = VK_NULL_HANDLE;
	vmaAllocCreateInfo.pUserData      = nullptr;
	vmaAllocCreateInfo.requiredFlags = 0;// VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	vmaAllocCreateInfo.preferredFlags = 0;// VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

	Buffer allocatedBuffer;
	ErrorCheck(vmaCreateBuffer(m_allocator,
		&vkAllocCreateInfo, &vmaAllocCreateInfo,
		&allocatedBuffer.m_vkBuffer, &allocatedBuffer.m_vmaAllocation, &allocatedBuffer.m_vmaAllocationInfo
	));

	allocatedBuffer.m_vkBufferUsage  = bufferUsageFlags;
	allocatedBuffer.m_vmaBufferUsage = vmaBufferUsage;

	return allocatedBuffer;
}


void BufferAllocator::freeBuffer(Buffer buffer)
{
	if (buffer.m_vmaAllocation != VK_NULL_HANDLE)
	{
		vmaFreeMemory(m_allocator, buffer.m_vmaAllocation);
	}
	buffer.m_vmaAllocation = VK_NULL_HANDLE;
	buffer.m_vmaAllocationInfo = VmaAllocationInfo{0};

	if (buffer.m_vkBuffer != VK_NULL_HANDLE)
	{
		vmaDestroyBuffer(m_allocator, buffer.m_vkBuffer, buffer.m_vmaAllocation);
	}
	buffer.m_vkBuffer = VK_NULL_HANDLE;
}

void* BufferAllocator::mapBuffer(Buffer buffer) const
{
	void* mappedData = nullptr;
	VkResult result = vmaMapMemory(m_allocator, buffer.m_vmaAllocation, &mappedData);
	assert(result == VK_SUCCESS);
	return mappedData;
}

void BufferAllocator::unmapBuffer(Buffer buffer) const
{
	vmaUnmapMemory(m_allocator, buffer.m_vmaAllocation);
}

BufferAllocator::~BufferAllocator()
{

}