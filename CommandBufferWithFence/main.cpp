#include "Renderer.h"

#include "Helper.h"

int main() {

	Renderer renderer;

	VkDevice& vkDevice = renderer.vkDevice;
	VkQueue&  vkQueue  = renderer.vkQueue;

	VkFence fence;
	VkFenceCreateInfo fence_create_info{};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(vkDevice, &fence_create_info, nullptr, &fence);

	VkCommandPool cmd_pool;
	VkCommandPoolCreateInfo cmd_pool_create_info{};
	cmd_pool_create_info.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_create_info.queueFamilyIndex	= renderer.graphicsFamilyIdx;
	cmd_pool_create_info.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;


	CHECK_ERROR(vkCreateCommandPool(vkDevice, &cmd_pool_create_info, nullptr, &cmd_pool));

	VkCommandBuffer cmd_buffer;
	VkCommandBufferAllocateInfo cmd_buffer_allocate_info{};
	cmd_buffer_allocate_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_buffer_allocate_info.commandPool		= cmd_pool;
	cmd_buffer_allocate_info.commandBufferCount = 1;
	cmd_buffer_allocate_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkAllocateCommandBuffers(vkDevice, &cmd_buffer_allocate_info, &cmd_buffer);

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(cmd_buffer, &begin_info);

	VkViewport viewport{};
	viewport.x			= 0;
	viewport.y			= 0;
	viewport.width		= 512;
	viewport.height		= 512;
	viewport.minDepth	= 0.0f;
	viewport.maxDepth	= 1.0f;


	vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

	vkEndCommandBuffer(cmd_buffer);

	VkSubmitInfo submit_info{};
	submit_info.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount	= 1;
	submit_info.pCommandBuffers		= &cmd_buffer;
	vkQueueSubmit(vkQueue, 1, &submit_info, fence);

	vkWaitForFences(vkDevice, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

	// wait for gpu
	vkQueueWaitIdle(vkQueue);

	// destroy fence
	vkDestroyFence(vkDevice, fence, nullptr);

	vkDestroyCommandPool(vkDevice, cmd_pool, nullptr);

	return 0;
}