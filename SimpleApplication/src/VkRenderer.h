#pragma once

// STD
#include <string>
#include <vector>

// Vulkan
#include <vulkan/vulkan.h>

class VkRenderer
{
public:
	VkRenderer();
	~VkRenderer();

private:
	void initInstance(const char* applicationName);
	void deInitInstance();

	void initDevice();
	void deInitDevice();

	void setupDebug();
	void initDebug();
	void deInitDebug();

	VkInstance			vkInstance				= VK_NULL_HANDLE;
	VkDevice			vkDevice				= VK_NULL_HANDLE;
	VkPhysicalDevice	vkGPU					= VK_NULL_HANDLE;
	uint32_t            vkGraphicsFamilyIndex	= 0;

	std::vector<const char*> vkLayerList;
	std::vector<const char*> vkExtensionsList;

	VkDebugReportCallbackEXT debugReport		= VK_NULL_HANDLE;
	VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo = {};
};

