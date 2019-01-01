#include "Buffer.h"

#include <assert.h>

#include <vk_mem_alloc.h>

Buffer::Buffer()
{

}

Buffer::~Buffer()
{
}

VkBuffer Buffer::getVkBuffer() const
{
	return m_vkBuffer;
}