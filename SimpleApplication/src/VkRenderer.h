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

	void init(const char* applicationName, const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& deviceExtensions);
	void createWindowSurface(GLFWwindow* windowPtr);
	void deInit();

	// get functions
	const VkInstance							getVkInstance()						const;
	const VkPhysicalDevice						getVkPhysicalDevice()				const;
	const VkDevice								getVkDevice()						const;
	const VkQueue								getVkQueue()						const;
	const uint32_t								getVkGraphicsQueueFamilyIndex()		const;
	const VkPhysicalDeviceProperties&			getVkPhysicalDeviceProperties()		const;
	const VkPhysicalDeviceMemoryProperties&		getVkPhysicalDeviceMemProperties()	const;

private:
	void initInstance(const char* applicationName);
	void deInitInstance();

	void initDevice(const std::vector<const char*>& deviceExtension);
	void deInitDevice();

	void setupDebug(const std::vector<const char*>& requiredExtensions);
	void initDebug();
	void deInitDebug();

	void initSwapChain();
	void deInitSwapChain();
	void initSwapChainImages();
	void deInitSwapChainImages();

	void initDepthStencilImage();
	void deInitDepthStencilImage();

	VkInstance							vkInstance				= VK_NULL_HANDLE;
	VkDevice							vkDevice				= VK_NULL_HANDLE;
	VkPhysicalDevice					vkGPU					= VK_NULL_HANDLE;
	VkPhysicalDeviceProperties			vkGPUProperties			= {};
	VkPhysicalDeviceMemoryProperties    vkGPUMemProperties		= {};
	uint32_t							vkGraphicsFamilyIndex	= 0;
	VkQueue								vkQueue					= VK_NULL_HANDLE;
	VkSurfaceKHR						vkSurface               = VK_NULL_HANDLE;
	VkSwapchainKHR						vkSwapChain				= VK_NULL_HANDLE;
	VkSurfaceCapabilitiesKHR			vkSurfaceCapabilities	= {};
	VkSurfaceFormatKHR					vkSurfaceFormat			= {};
	uint32_t							vkSurfaceWidth			= UINT32_MAX;
	uint32_t							vkSurfaceHeight			= UINT32_MAX;
	uint32_t							vkSwapChainImageCount   = 2;

	// Swap chain images
	std::vector<VkImage>				vkSwapChainImages;
	std::vector<VkImageView>			vkSwapChainImageViews;

	// Depth Stencil Image and Image View
	bool								vkStencilBufferAvailable = false;
	VkFormat							vkDepthStencilFormat	= VK_FORMAT_UNDEFINED;
	VkImage								vkDepthStencilImage		= VK_NULL_HANDLE;
	VkDeviceMemory						vkDepthStencilImageMem  = VK_NULL_HANDLE;
	VkImageView							vkDepthStencilImageView	= VK_NULL_HANDLE;


	// Layres and extensions
	std::vector<const char*>			vkLayerList;
	std::vector<const char*>			vkExtensionsList;
	

	VkDebugReportCallbackEXT			debugReport						= VK_NULL_HANDLE;
	VkDebugReportCallbackCreateInfoEXT	debugReportCallbackCreateInfo	= {};
};

