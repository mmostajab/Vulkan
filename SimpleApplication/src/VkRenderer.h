#pragma once

// STD
#include <string>
#include <vector>

// Vulkan
#include <vulkan/vulkan.h>

#ifdef _WIN32
// Windows
#include <Windows.h>
#endif

// GLFW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VkRenderer
{
public:
	VkRenderer();
	~VkRenderer();

	void init(const char* applicationName, const std::vector<const char*>& requiredExtensions);
	void createWindowSurface(GLFWwindow* windowPtr);
	void deInit();

	// get functions
	const VkInstance					getVkInstance()					const;
	const VkPhysicalDevice				getVkPhysicalDevice()			const;
	const VkDevice						getVkDevice()					const;
	const VkQueue						getVkQueue()					const;
	const uint32_t						getVkGraphicsQueueFamilyIndex() const;
	const VkPhysicalDeviceProperties&	getVkPhysicalDeviceProperties() const;

private:
	void initInstance(const char* applicationName);
	void deInitInstance();

	void initDevice();
	void deInitDevice();

	void setupDebug(const std::vector<const char*>& requiredExtensions);
	void initDebug();
	void deInitDebug();

	VkInstance			vkInstance				= VK_NULL_HANDLE;
	VkDevice			vkDevice				= VK_NULL_HANDLE;
	VkPhysicalDevice	vkGPU					= VK_NULL_HANDLE;
	VkPhysicalDeviceProperties vkGPUProperties  = {};
	uint32_t            vkGraphicsFamilyIndex	= 0;
	VkQueue             vkQueue					= VK_NULL_HANDLE;
	VkSurfaceKHR		vkSurface               = VK_NULL_HANDLE;

	std::vector<const char*> vkLayerList;
	std::vector<const char*> vkExtensionsList;

	VkDebugReportCallbackEXT debugReport		= VK_NULL_HANDLE;
	VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo = {};
};

