#pragma once
class VkHelper
{
public:
	VkHelper() = delete;
	~VkHelper() = delete;

	static uint32_t findMemoryType(const VkPhysicalDevice physicalDevice, const VkMemoryRequirements& memReqs, VkMemoryPropertyFlags reqFlags, VkMemoryPropertyFlags prefFlags);
	static VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	static bool hasStencilComponent(VkFormat format);

	static void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags preferedPropertyFlags, VkMemoryPropertyFlags requiredPropertyFlags, VkBuffer* bufferId, VkDeviceMemory* bufferMem);
	static void create2DImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags reqFlags, VkMemoryPropertyFlags preferredFlags, VkImage* img, VkDeviceMemory* deviceImgMem);
	static void create2DImageView(VkDevice device, VkImage img, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imgView);
	static void copyBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkBuffer srcBufferId, VkBuffer dstBufferId, VkDeviceSize size);
	static void copyImage(VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkImage srcImageId, VkImage dstImageId, uint32_t width, uint32_t height);
	static void transitImageLayout(VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkImage img, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	static VkCommandBuffer beginSingleTimeCommandBufffer(VkDevice device, VkCommandPool cmdPool);
	static void endSingleTimeCommandBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkCommandBuffer cmdBuffer);
};

