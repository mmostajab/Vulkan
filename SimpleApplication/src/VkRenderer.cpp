#include "VkRenderer.h"
#include "helper.h"

// STD
#include <iostream>
#include <assert.h>
#include <vector>
#include <sstream>



VkRenderer::VkRenderer()
{
}


VkRenderer::~VkRenderer()
{
}

void VkRenderer::init(const char* applicationName, const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& deviceExtensions)
{
	setupDebug(instanceExtensions);
	initInstance(applicationName);
	initDebug();
	initDevice(deviceExtensions);
}

void VkRenderer::createWindowSurface(GLFWwindow * windowPtr)
{
	ErrorCheck(glfwCreateWindowSurface(vkInstance, windowPtr, nullptr, &vkSurface));

	VkBool32 WSISupported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(vkGPU, vkGraphicsFamilyIndex, vkSurface, &WSISupported);
	if (!WSISupported) {
		assert(0 && "WSI not supported.");
		std::exit(-1);
	}
		
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkGPU, vkSurface, &vkSurfaceCapabilities);
	if (vkSurfaceCapabilities.currentExtent.width < UINT32_MAX && vkSurfaceCapabilities.currentExtent.height < UINT32_MAX) {
		vkSurfaceWidth  = vkSurfaceCapabilities.currentExtent.width;
		vkSurfaceHeight = vkSurfaceCapabilities.currentExtent.height;
	}

	uint32_t surfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(vkGPU, vkSurface, &surfaceFormatCount, nullptr);
	if (surfaceFormatCount == 0) {
		assert(0 && "Surface does not support any formats.");
		exit(-1);
	}
	std::vector<VkSurfaceFormatKHR> surfaceFormat(surfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(vkGPU, vkSurface, &surfaceFormatCount, surfaceFormat.data());
	if (surfaceFormat[0].format == VK_FORMAT_UNDEFINED) {
		vkSurfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
		vkSurfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	}
	else {
		vkSurfaceFormat = surfaceFormat[0];
	}

	initSwapChain();
	initSwapChainImages();
	initDepthStencilImage();
}

void VkRenderer::deInit() 
{
	deInitDepthStencilImage();
	deInitSwapChainImages();
	deInitSwapChain();
	vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
	deInitDevice();
	deInitDebug();
	deInitInstance();
}

const VkInstance VkRenderer::getVkInstance() const
{
	return vkInstance;
}

const VkPhysicalDevice VkRenderer::getVkPhysicalDevice() const
{
	return vkGPU;
}

const VkDevice VkRenderer::getVkDevice() const
{
	return vkDevice;
}

const VkQueue VkRenderer::getVkQueue() const
{
	return vkQueue;
}

const uint32_t VkRenderer::getVkGraphicsQueueFamilyIndex() const
{
	return vkGraphicsFamilyIndex;
}

const VkPhysicalDeviceProperties & VkRenderer::getVkPhysicalDeviceProperties() const
{
	return vkGPUProperties;
}

const VkPhysicalDeviceMemoryProperties & VkRenderer::getVkPhysicalDeviceMemProperties() const
{
	return vkGPUMemProperties;
}

void VkRenderer::initInstance(const char* applicationName)
{
	VkApplicationInfo applicationInfo{};
	applicationInfo.sType      = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 21);
	applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	applicationInfo.pApplicationName = applicationName;

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(vkExtensionsList.size());
	instanceCreateInfo.ppEnabledExtensionNames = vkExtensionsList.data();
	instanceCreateInfo.enabledLayerCount       = static_cast<uint32_t>(vkLayerList.size());
	instanceCreateInfo.ppEnabledLayerNames     = vkLayerList.data();

	// Magically, make you able to debug instance creation.
	instanceCreateInfo.pNext                   = &debugReportCallbackCreateInfo;

	ErrorCheck( vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance) );
}

void VkRenderer::deInitInstance()
{
	vkDestroyInstance(vkInstance, nullptr);
	vkInstance = nullptr;
}

static std::string getDeviceTypeString(VkPhysicalDeviceType deviceType) {
	std::string deviceTypeString;
	switch (deviceType) {
	case VK_PHYSICAL_DEVICE_TYPE_OTHER:
		deviceTypeString = "Other";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		deviceTypeString = "Integrated GPU";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		deviceTypeString = "Discrete GPU";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		deviceTypeString = "Virtual GPU";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_CPU:
		deviceTypeString = "CPU";
		break;
	default:
		deviceTypeString = "Type not implemented";
	}

	return deviceTypeString;
}

std::string getAPIVersionString(uint32_t apiVersion){
	std::ostringstream apiVersionString;

	apiVersionString << VK_VERSION_MAJOR(apiVersion);
	apiVersionString << ".";
	apiVersionString << VK_VERSION_MINOR(apiVersion);
	apiVersionString << ".";
	apiVersionString << VK_VERSION_PATCH(apiVersion);
	
	return apiVersionString.str();
}

void VkRenderer::initDevice(const std::vector<const char*>& deviceExtensions)
{
	// ========================================
	// Physical Devices extraction
	// ========================================
	uint32_t numPhysicalDevices = 0;
	vkEnumeratePhysicalDevices(vkInstance, &numPhysicalDevices, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices(numPhysicalDevices);
	vkEnumeratePhysicalDevices(vkInstance, &numPhysicalDevices, physicalDevices.data());
	vkGPU = physicalDevices[0];

	// ========================================
	// Physical Device Properties extraction
	// ========================================
	vkGetPhysicalDeviceProperties(vkGPU, &vkGPUProperties);
	std::cout << "Device Name = " << vkGPUProperties.deviceName << std::endl;
	std::cout << "Device Type = " << getDeviceTypeString(vkGPUProperties.deviceType) << std::endl;
	std::cout << "Device API version = " << getAPIVersionString(vkGPUProperties.apiVersion) << std::endl;
	
	// =============================================
	// Physical Device Memory Properties extraction
	// =============================================
	vkGetPhysicalDeviceMemoryProperties(vkGPU, &vkGPUMemProperties);

	// ========================================
	// Physical Device Queue Family extraction
	// ========================================
#if 1
	uint32_t numPhysicalDeviceQueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(vkGPU, &numPhysicalDeviceQueueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> physicalDeviceQueueFamilyPropertiesList(numPhysicalDeviceQueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(vkGPU, &numPhysicalDeviceQueueFamilyCount, physicalDeviceQueueFamilyPropertiesList.data());
	bool found = false;
	for (uint32_t i = 0; i < physicalDeviceQueueFamilyPropertiesList.size(); i++) {
#if 0
		if (physicalDeviceQueueFamilyPropertiesList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			found = true;
			vkGraphicsFamilyIndex = i;
			break;
		}
#endif
		if (glfwGetPhysicalDevicePresentationSupport(vkInstance, vkGPU, i) == GLFW_TRUE) {
			found = true;
			vkGraphicsFamilyIndex = i;
			break;
		}
	}

	if (!found) {
		assert(0 && "Vulkan ERROR: Queue family supporting graphics not found.");
		std::exit(-1);
	}
#endif

	// ==================================================
	// Layers extraction: Instance
	// ==================================================
	uint32_t instanceLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
	std::vector<VkLayerProperties> instanceLayerPropertyList(instanceLayerCount);
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerPropertyList.data());
	// Output available layers
	/*std::cout << "Instance Layers " << std::endl;
	for (auto& i : instanceLayerPropertyList) {
		std::cout << "\t" << i.layerName << "\t | " << i.description << std::endl;
	}
	std::cout << std::endl;*/

	// ========================================
	// Queue Create Information
	// ========================================
	float queuePrioritiesList[1] = { 1.0 };
	VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
	deviceQueueCreateInfo.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.queueFamilyIndex	= vkGraphicsFamilyIndex;
	deviceQueueCreateInfo.queueCount		= 1;
	deviceQueueCreateInfo.pQueuePriorities	= queuePrioritiesList;

	// ========================================
	// Device creation
	// ========================================
	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount		= 1;
	deviceCreateInfo.pQueueCreateInfos			= &deviceQueueCreateInfo;
	deviceCreateInfo.enabledExtensionCount		= static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames	= deviceExtensions.data();
	//deviceCreateInfo.enabledLayerCount		= static_cast<uint32_t>(vkLayerList.size());
	//deviceCreateInfo.ppEnabledLayerNames		= vkLayerList.data();

	ErrorCheck( vkCreateDevice(vkGPU, &deviceCreateInfo, nullptr, &vkDevice) );

	vkGetDeviceQueue(vkDevice, vkGraphicsFamilyIndex, 0, &vkQueue);
}

void VkRenderer::deInitDevice()
{
	vkDestroyDevice(vkDevice, nullptr);
}

VKAPI_ATTR VkBool32 VulkanDebugCallBackFunc(
	VkDebugReportFlagsEXT                       flags,
	VkDebugReportObjectTypeEXT                  objectType,
	uint64_t                                    object,
	size_t                                      location,
	int32_t                                     messageCode,
	const char*                                 pLayerPrefix,	// Which layer calls this call back
	const char*                                 pMessage,
	void*                                       pUserData) {

	std::ostringstream msg;

	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		msg << "INFO:\t";
	} 
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		msg << "WARN:\t";
	}
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		msg << "PERF_WARN: ";
	}
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		msg << "ERROR:\t";
	}
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		msg << "DEBUG:\t";
	}

	msg << "@[" << pLayerPrefix << "] " << pMessage;

	std::cout << msg.str() << std::endl; 

#ifdef _WIN32
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		MessageBoxA(0, msg.str().c_str(), "ERROR", MB_OK);
	}
#endif

	//return true;  // Stops here.
	return false; // Go to the upper layer.
}

void VkRenderer::setupDebug(const std::vector<const char*>& requiredExtensions)
{


	//vkLayerList.push_back("VK_LAYER_LUNARG_api_dump");
	vkLayerList.push_back("VK_LAYER_LUNARG_core_validation");
	//vkLayerList.push_back("VK_LAYER_LUNARG_monitor");
	vkLayerList.push_back("VK_LAYER_GOOGLE_threading");
	vkLayerList.push_back("VK_LAYER_LUNARG_swapchain");
	vkLayerList.push_back("VK_LAYER_LUNARG_image");
	vkLayerList.push_back("VK_LAYER_LUNARG_object_tracker");
	vkLayerList.push_back("VK_LAYER_GOOGLE_unique_objects");
	vkLayerList.push_back("VK_LAYER_LUNARG_parameter_validation");
	//vkLayerList.push_back("VK_LAYER_LUNARG_screenshot");
	//vkLayerList.push_back("VK_LAYER_LUNARG_vktrace");
	//vkLayerList.push_back("VK_LAYER_RENDERDOC_Capture");
	//vkLayerList.push_back("VK_LAYER_LUNARG_standard_validation");

	vkExtensionsList.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	vkExtensionsList.insert(vkExtensionsList.end(), requiredExtensions.begin(), requiredExtensions.end());

	debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugReportCallbackCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)VulkanDebugCallBackFunc;
	debugReportCallbackCreateInfo.flags =
		//VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT;// |
		//VK_DEBUG_REPORT_DEBUG_BIT_EXT;
}

PFN_vkCreateDebugReportCallbackEXT	fvkCreateDebugReportCallBackExt = nullptr;
PFN_vkDestroyDebugReportCallbackEXT	fvkDestroyDebugReportCallBackExt = nullptr;

void VkRenderer::initDebug()
{
	fvkCreateDebugReportCallBackExt  = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");
	fvkDestroyDebugReportCallBackExt = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
	if (fvkCreateDebugReportCallBackExt == nullptr || fvkDestroyDebugReportCallBackExt == nullptr) {
		assert(0 && "Vulkan ERROR: cannot fetch debug function pointers.");
		exit(-1);
	}	

	fvkCreateDebugReportCallBackExt(vkInstance, &debugReportCallbackCreateInfo, nullptr, &debugReport);
}

void VkRenderer::deInitDebug()
{
	fvkDestroyDebugReportCallBackExt(vkInstance, debugReport, nullptr);
	debugReport = VK_NULL_HANDLE;
}

void VkRenderer::initSwapChain()
{
	if (vkSwapChainImageCount > vkSurfaceCapabilities.maxImageCount) vkSwapChainImageCount = vkSurfaceCapabilities.maxImageCount;
	if (vkSwapChainImageCount < vkSurfaceCapabilities.minImageCount) vkSwapChainImageCount = vkSurfaceCapabilities.minImageCount;

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(vkGPU, vkSurface, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(vkGPU, vkSurface, &presentModeCount, presentModes.data());
	for (auto& pMode : presentModes) {
		if (pMode == VK_PRESENT_MODE_MAILBOX_KHR)
			presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	}

	VkSwapchainCreateInfoKHR swapChainCreateInfo{};
	swapChainCreateInfo.sType					= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface					= vkSurface;
	swapChainCreateInfo.minImageCount			= vkSwapChainImageCount;					// at least Double buffering
	swapChainCreateInfo.imageFormat				= vkSurfaceFormat.format;
	swapChainCreateInfo.imageExtent.width		= vkSurfaceWidth;
	swapChainCreateInfo.imageExtent.height		= vkSurfaceHeight;
	swapChainCreateInfo.imageArrayLayers		= 1;										// 1: normal, 2: stereoscopic
	swapChainCreateInfo.imageUsage				= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.imageSharingMode		= VK_SHARING_MODE_EXCLUSIVE;
	swapChainCreateInfo.queueFamilyIndexCount	= 0;
	swapChainCreateInfo.pQueueFamilyIndices		= VK_NULL_HANDLE;
	swapChainCreateInfo.preTransform			= VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;	// Useful for Android
	swapChainCreateInfo.compositeAlpha			= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.presentMode				= presentMode;
	swapChainCreateInfo.clipped					= VK_TRUE;
	swapChainCreateInfo.oldSwapchain			= VK_NULL_HANDLE;							// Useful during resizing.


	vkCreateSwapchainKHR(vkDevice, &swapChainCreateInfo, nullptr, &vkSwapChain);

	vkGetSwapchainImagesKHR(vkDevice, vkSwapChain, &vkSwapChainImageCount, nullptr);
}

void VkRenderer::deInitSwapChain()
{
	vkDestroySwapchainKHR(vkDevice, vkSwapChain, nullptr);
	vkSwapChain = VK_NULL_HANDLE;
}

void VkRenderer::initSwapChainImages()
{
	vkSwapChainImages.resize(vkSwapChainImageCount);
	vkSwapChainImageViews.resize(vkSwapChainImageCount);

	ErrorCheck( vkGetSwapchainImagesKHR(vkDevice, vkSwapChain, &vkSwapChainImageCount, vkSwapChainImages.data()) );

	for (uint32_t i = 0; i < vkSwapChainImageCount; i++) {
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType							= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image							= vkSwapChainImages[i];
		imageViewCreateInfo.viewType						= VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format							= vkSurfaceFormat.format;
		imageViewCreateInfo.components.r					= VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g					= VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b					= VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel	= 0;
		imageViewCreateInfo.subresourceRange.levelCount		= 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount		= 1;
		
		ErrorCheck( vkCreateImageView(vkDevice, &imageViewCreateInfo, nullptr, &vkSwapChainImageViews[i]) );
	}
}

void VkRenderer::deInitSwapChainImages()
{
	for(auto& view : vkSwapChainImageViews)
		vkDestroyImageView(vkDevice, view, nullptr);
}

void VkRenderer::initDepthStencilImage()
{
	// ==========================================
	// Detect porper depth stencil image format
	// ==========================================

	std::vector<VkFormat> try_formats{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM
	};

	for (auto f : try_formats) {
		VkFormatProperties formatProperties{};
		vkGetPhysicalDeviceFormatProperties(vkGPU, f, &formatProperties);
		if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			vkDepthStencilFormat = f;
			break;
		}
	}

	if (vkDepthStencilFormat == VK_FORMAT_UNDEFINED) {
		assert(0 && "Depth Stencil format is undefined.");
		exit(-1);
	}


	if (
		vkDepthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
		vkDepthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
		vkDepthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT ||
		vkDepthStencilFormat == VK_FORMAT_S8_UINT
		) {
		vkStencilBufferAvailable = true;
	}

	// ===============================
	// Create Image Handle
	// ===============================

	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType					= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.flags					= 0;
	imageCreateInfo.imageType				= VK_IMAGE_TYPE_2D;
	imageCreateInfo.format					= vkDepthStencilFormat;
	imageCreateInfo.extent.width			= vkSurfaceWidth;
	imageCreateInfo.extent.height			= vkSurfaceHeight;
	imageCreateInfo.extent.depth			= 1;
	imageCreateInfo.mipLevels				= 1;
	imageCreateInfo.arrayLayers				= 1;
	imageCreateInfo.samples					= VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling					= VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage					= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCreateInfo.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount	= VK_QUEUE_FAMILY_IGNORED;
	imageCreateInfo.pQueueFamilyIndices		= nullptr;
	imageCreateInfo.initialLayout			= VK_IMAGE_LAYOUT_UNDEFINED;

	
	vkCreateImage(vkDevice, &imageCreateInfo, nullptr, &vkDepthStencilImage);

	// ==================================
	// Allocate Memory for the Image
	// ==================================
	VkMemoryRequirements imageMemRequirements{};
	vkGetImageMemoryRequirements(vkDevice, vkDepthStencilImage, &imageMemRequirements);
	
	VkMemoryPropertyFlags requiredProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	uint32_t	vkMemTypeIndex = FindVkMemoryTypeIndex(vkGPUMemProperties, imageMemRequirements, requiredProperties);

	VkMemoryAllocateInfo memAllocInfo{};
	memAllocInfo.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize		= imageMemRequirements.size;
	memAllocInfo.memoryTypeIndex	= vkMemTypeIndex;

	vkAllocateMemory(vkDevice, &memAllocInfo, nullptr, &vkDepthStencilImageMem);
	vkBindImageMemory(vkDevice, vkDepthStencilImage, vkDepthStencilImageMem, 0);

	// ============================================
	// Create Image View for Depth Stencil Images
	// ============================================

	VkImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.sType								= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image								= vkDepthStencilImage;
	imageViewCreateInfo.viewType							= VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format								= vkDepthStencilFormat;
	imageViewCreateInfo.components.r						= VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g						= VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b						= VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask			= VK_IMAGE_ASPECT_DEPTH_BIT | (vkStencilBufferAvailable ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
	imageViewCreateInfo.subresourceRange.baseMipLevel		= 0;
	imageViewCreateInfo.subresourceRange.levelCount			= 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer		= 0;
	imageViewCreateInfo.subresourceRange.layerCount			= 1;

	vkCreateImageView(vkDevice, &imageViewCreateInfo, nullptr, &vkDepthStencilImageView);
}

void VkRenderer::deInitDepthStencilImage()
{
	vkDestroyImageView(vkDevice, vkDepthStencilImageView, nullptr);
	vkFreeMemory(vkDevice, vkDepthStencilImageMem, nullptr);
	vkDestroyImage(vkDevice, vkDepthStencilImage, nullptr);
}
