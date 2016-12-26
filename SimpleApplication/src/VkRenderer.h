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

	VkInstance			vkInstance = nullptr;
	VkDevice			vkDevice   = nullptr;
	VkPhysicalDevice	vkGPU      = nullptr;
	uint32_t            vkGraphicsFamilyIndex = 0;

	std::vector<const char*> vkValidationLayerList;
	std::vector<const char*> vkExtensionsList;

	VkDebugReportCallbackEXT debugReport = nullptr;
	VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo = {};
};

