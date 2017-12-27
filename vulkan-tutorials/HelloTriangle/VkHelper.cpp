#include "stdafx.h"
#include "VkHelper.h"

uint32_t VkHelper::findMemoryType(const VkPhysicalDevice physicalDevice, const VkMemoryRequirements& memReqs, VkMemoryPropertyFlags reqFlags, VkMemoryPropertyFlags prefFlags)
{
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

	uint32_t selectedType = ~0u;
	uint32_t memType;

	for (memType = 0; memType < memProps.memoryTypeCount; ++memType)
	{
		if (memReqs.memoryTypeBits & (1 << memType))
		{
			const VkMemoryType& type = memProps.memoryTypes[memType];

			if ((type.propertyFlags & prefFlags) == prefFlags)
			{
				selectedType = memType;
				break;
			}
		}
	}

	if (selectedType == ~0u)
	{
		for (memType = 0; memType < memProps.memoryTypeCount; ++memType)
		{
			if (memReqs.memoryTypeBits & (1 << memType))
			{
				const VkMemoryType& type = memProps.memoryTypes[memType];

				if ((type.propertyFlags & reqFlags) == reqFlags)
				{
					selectedType = memType;
					break;
				}
			}
		}
	}

	if (selectedType == ~0u)
	{
		std::runtime_error("ERROR: required memory type is not found!");
	}

	return selectedType;
}

VkFormat VkHelper::findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (auto format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			return format;
	}

	throw std::runtime_error("ERROR: Failed to find supported format.");

	return VkFormat();
}

bool VkHelper::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VkHelper::createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags preferedPropertyFlags, VkMemoryPropertyFlags requiredPropertyFlags, VkBuffer* bufferId, VkDeviceMemory* bufferMem)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = VK_NULL_HANDLE;
	bufferInfo.flags = 0;
	bufferInfo.size = bufferSize;// mesh->getVertexCount() * mesh->getVertexStride();
	bufferInfo.usage = usageFlags;// VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.pQueueFamilyIndices = nullptr;
	bufferInfo.queueFamilyIndexCount = 0;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, bufferId) != VK_SUCCESS)
		std::runtime_error("failed to create vertex buffer.");

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(device, *bufferId, &memReq);
	//VkMemoryPropertyFlags required = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	//VkMemoryPropertyFlags prefered = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	uint32_t memType = findMemoryType(physicalDevice, memReq, requiredPropertyFlags, preferedPropertyFlags);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = VK_NULL_HANDLE;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = memType;

	if (vkAllocateMemory(device, &allocInfo, nullptr, bufferMem) != VK_SUCCESS)
		std::runtime_error("ERROR: Allocating buffer memory failed.");

	if (vkBindBufferMemory(device, *bufferId, *bufferMem, 0) != VK_SUCCESS)
		std::runtime_error("ERROR: Binding Memory to Buffer Failed.");
}

void VkHelper::create2DImage(VkDevice device, VkPhysicalDevice physicalDevice, 
	uint32_t width, uint32_t height, 
	VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags reqFlags, VkMemoryPropertyFlags preferredFlags,
	VkImage* img, VkDeviceMemory* deviceImgMem)
{
	VkImageCreateInfo imgCreateInfo{};
	imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imgCreateInfo.pNext = VK_NULL_HANDLE;
	imgCreateInfo.flags = 0;
	imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imgCreateInfo.extent.width = width;
	imgCreateInfo.extent.height = height;
	imgCreateInfo.extent.depth = 1;
	imgCreateInfo.arrayLayers = 1;
	imgCreateInfo.mipLevels = 1;
	imgCreateInfo.format = format;
	imgCreateInfo.tiling = tiling;
	imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	imgCreateInfo.queueFamilyIndexCount = -1;
	imgCreateInfo.pQueueFamilyIndices = 0;
	imgCreateInfo.usage = usage;
	imgCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	if (vkCreateImage(device, &imgCreateInfo, nullptr, img) != VK_SUCCESS)
		std::runtime_error("ERROR: Failed to create image.");

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(device, *img, &memReqs);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = VK_NULL_HANDLE;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = VkHelper::findMemoryType(physicalDevice, memReqs, reqFlags, preferredFlags);

	if (vkAllocateMemory(device, &allocInfo, nullptr, deviceImgMem) != VK_SUCCESS)
		std::runtime_error("ERROR: Failed to allocate memory.");

	if (vkBindImageMemory(device, *img, *deviceImgMem, 0) != VK_SUCCESS)
		std::runtime_error("ERROR: Failed to bind image memory.");
}

void VkHelper::create2DImageView(VkDevice device, VkImage img, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView * imgView)
{
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.pNext = VK_NULL_HANDLE;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.image = img;
	createInfo.format = format;
	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	if (vkCreateImageView(device, &createInfo, nullptr, imgView))
	{
		throw std::runtime_error("ERROR: Cannot Create Image View.");
	}
}

void VkHelper::copyBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkBuffer srcBufferId, VkBuffer dstBufferId, VkDeviceSize size)
{
	VkCommandBuffer cmdBuffer = beginSingleTimeCommandBufffer(device, cmdPool);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(cmdBuffer, srcBufferId, dstBufferId, 1, &copyRegion);

	endSingleTimeCommandBuffer(device, cmdPool, queue, cmdBuffer);
}

void VkHelper::copyImage(VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkImage srcImageId, VkImage dstImageId, uint32_t width, uint32_t height)
{
	VkCommandBuffer cmdBuffer = beginSingleTimeCommandBufffer(device, cmdPool);

	VkImageSubresourceLayers subResource = {};
	subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResource.baseArrayLayer = 0;
	subResource.layerCount = 1;
	subResource.mipLevel = 0;

	VkImageCopy region{};
	region.srcOffset = { 0, 0, 0 };
	region.srcSubresource = subResource;
	region.dstOffset = { 0, 0, 0 };
	region.dstSubresource = subResource;
	region.extent.width = width;
	region.extent.height = height;
	region.extent.depth = 1;

	vkCmdCopyImage(
		cmdBuffer,
		srcImageId, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dstImageId, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &region);

	endSingleTimeCommandBuffer(device, cmdPool, queue, cmdBuffer);
}

void VkHelper::transitImageLayout(VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkImage img, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer cmdBuffer = beginSingleTimeCommandBufffer(device, cmdPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = VK_NULL_HANDLE;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = img;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (hasStencilComponent(format))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} 
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	auto isInInitialState = [oldLayout]() -> bool
	{
		return oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED || oldLayout == VK_IMAGE_LAYOUT_UNDEFINED;
	};

	VkPipelineStageFlags srcStageFlag, dstStageFlag;

	if (isInInitialState() && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		srcStageFlag = VK_PIPELINE_STAGE_HOST_BIT;
		dstStageFlag = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (isInInitialState() && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		srcStageFlag = VK_PIPELINE_STAGE_HOST_BIT;
		dstStageFlag = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		srcStageFlag = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStageFlag = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (isInInitialState() && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		srcStageFlag = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dstStageFlag = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	}
	else
	{
		throw std::runtime_error("ERROR: Not supported layouts.");
	}

	vkCmdPipelineBarrier(
		cmdBuffer,
		srcStageFlag, dstStageFlag,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	endSingleTimeCommandBuffer(device, cmdPool, queue, cmdBuffer);
}


VkCommandBuffer VkHelper::beginSingleTimeCommandBufffer(VkDevice device, VkCommandPool cmdPool)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = VK_NULL_HANDLE;
	allocInfo.commandPool = cmdPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer cmdBuffer;
	if (vkAllocateCommandBuffers(device, &allocInfo, &cmdBuffer) != VK_SUCCESS)
		std::runtime_error("ERROR: Cannot allocate command buffers.");

	VkCommandBufferBeginInfo cmdBufferBeginInfo{};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.pNext = VK_NULL_HANDLE;
	cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	cmdBufferBeginInfo.pInheritanceInfo = 0;

	if (vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo) != VK_SUCCESS)
		std::runtime_error("ERROR: Command buffer begin failed.");

	return cmdBuffer;
}

void VkHelper::endSingleTimeCommandBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkCommandBuffer cmdBuffer)
{
	if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS)
		std::runtime_error("ERROR: Command buffer end failed.");

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = VK_NULL_HANDLE;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;
	
	if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		std::runtime_error("ERROR: Submission to the queue failed.");

	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuffer);
}

