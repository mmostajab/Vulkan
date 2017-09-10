#include "VkRenderer.h"
#include "helper.h"

// STD
#include <iostream>
#include <assert.h>
#include <vector>
#include <sstream>
#include <array>



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
	initRenderPass();
	initFrameBuffer();
	initSynchronization();
}

void VkRenderer::destroySurface() {
	deInitSynchronization();
	deInitFrameBuffer();
	deInitRenderPass();
	deInitDepthStencilImage();
	deInitSwapChainImages();
	deInitSwapChain();

	vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
}

void VkRenderer::deInit() 
{
	// Wait until the commands in the queue are done before starting the deinitialization.
	vkQueueWaitIdle(vkQueue);

	deInitSynchronization();
	deInitFrameBuffer();
	deInitRenderPass();
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

const VkSwapchainKHR & VkRenderer::getVkSwapChain() const
{
	return vkSwapChain;
}

const VkRenderPass & VkRenderer::getVkRenderPass() const
{
	return vkRenderPass;
}

const VkFramebuffer & VkRenderer::getVkActiveFrameBuffer() const
{
	return vkFrameBuffer[vkActiveSwapChainID];
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
	deviceCreateInfo.enabledLayerCount			= 0;
	deviceCreateInfo.ppEnabledLayerNames		= nullptr;
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

//#define ENABLE_VULKAN_DEBUGGING_LAYERS
void VkRenderer::setupDebug(const std::vector<const char*>& requiredExtensions)
{
	uint32_t instanceLayerPropertiesCount = 0;
	vkEnumerateInstanceLayerProperties(&instanceLayerPropertiesCount, nullptr);
	vkLayerProps.resize(instanceLayerPropertiesCount);
	vkEnumerateInstanceLayerProperties(&instanceLayerPropertiesCount, vkLayerProps.data());

	std::cout << "Instance layers: " << std::endl;
	for (auto& instLayerProps : vkLayerProps) {
		std::cout << instLayerProps.layerName << "\t[x]\n";
		//vkLayerList.push_back(instLayerProps.layerName);
	}

#ifdef ENABLE_VULKAN_DEBUGGING_LAYERS
	vkLayerList.push_back("VK_LAYER_LUNARG_api_dump");
	vkLayerList.push_back("VK_LAYER_LUNARG_core_validation");
	vkLayerList.push_back("VK_LAYER_LUNARG_monitor");
	vkLayerList.push_back("VK_LAYER_LUNARG_object_tracker");
	vkLayerList.push_back("VK_LAYER_LUNARG_parameter_validation");
	vkLayerList.push_back("VK_LAYER_GOOGLE_threading");
	vkLayerList.push_back("VK_LAYER_GOOGLE_unique_objects");
	vkLayerList.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

	//vkLayerList.push_back("VK_LAYER_LUNARG_vktrace");
	//vkLayerList.push_back("VK_LAYER_RENDERDOC_Capture");
	

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
	uint32_t queueFamilyIndex = getVkGraphicsQueueFamilyIndex();
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
	imageCreateInfo.queueFamilyIndexCount	= 1;
	imageCreateInfo.pQueueFamilyIndices		= &queueFamilyIndex;
	imageCreateInfo.initialLayout			= VK_IMAGE_LAYOUT_UNDEFINED;

	
	ErrorCheck( vkCreateImage(vkDevice, &imageCreateInfo, nullptr, &vkDepthStencilImage) );

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

void VkRenderer::initRenderPass()
{
	// =====================================
	// Create Render Pass Attachments Array
	// =====================================
	std::array<VkAttachmentDescription, 2> attachments{};

	// Depth Stencil Buffer
	attachments[0].flags			= 0;
	attachments[0].format			= vkDepthStencilFormat;
	attachments[0].samples			= VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Color Buffer
	attachments[1].flags			= 0;
	attachments[1].format			= vkSurfaceFormat.format;
	attachments[1].samples			= VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// =====================================
	// Create Render Pass Sub Passes
	// =====================================
	std::array<VkAttachmentReference, 1> subpass0ColorAttachments{};
	subpass0ColorAttachments[0].attachment	= 1;
	subpass0ColorAttachments[0].layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference subpass0DepthStencilAttachment;
	subpass0DepthStencilAttachment.attachment	= 0;
	subpass0DepthStencilAttachment.layout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::array<VkSubpassDescription, 1> subPasses{};
	subPasses[0].pipelineBindPoint			= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPasses[0].inputAttachmentCount		= 0;		// Could be used for G-Buffer
	subPasses[0].pInputAttachments			= nullptr;
	subPasses[0].colorAttachmentCount		= static_cast<uint32_t>(subpass0ColorAttachments.size());
	subPasses[0].pColorAttachments			= subpass0ColorAttachments.data();
	subPasses[0].pDepthStencilAttachment    = &subpass0DepthStencilAttachment;

	// =====================================
	// Create Render Pass
	// =====================================
	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount	= static_cast<uint32_t>(attachments.size());
	renderPassCreateInfo.pAttachments		= attachments.data();
	renderPassCreateInfo.subpassCount		= static_cast<uint32_t>(subPasses.size());
	renderPassCreateInfo.pSubpasses			= subPasses.data();
	renderPassCreateInfo.dependencyCount	= 0;			// Can be used for G-Buffers.
	renderPassCreateInfo.pDependencies		= nullptr;

	ErrorCheck( vkCreateRenderPass(vkDevice, &renderPassCreateInfo, nullptr, &vkRenderPass) );
}

void VkRenderer::deInitRenderPass()
{
	vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
}

void VkRenderer::initFrameBuffer()
{
	// ====================================================
	// Create one frame buffer for each swap chain image
	// ====================================================
	vkFrameBuffer.resize(vkSwapChainImages.size());
	for (uint32_t i = 0; i < vkSwapChainImageCount; i++) {
		// Create attachments list for frame buffer creation
		// ! Needs to be compatiable with render pass
		std::array<VkImageView, 2> attachments;
		attachments[0] = vkDepthStencilImageView;
		attachments[1] = vkSwapChainImageViews[i];

		VkFramebufferCreateInfo frameBufferCreateInfo{};
		frameBufferCreateInfo.sType				= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.renderPass		= vkRenderPass;
		frameBufferCreateInfo.attachmentCount   = static_cast<uint32_t>(attachments.size());
		frameBufferCreateInfo.pAttachments		= attachments.data();
		frameBufferCreateInfo.width				= vkSurfaceWidth;
		frameBufferCreateInfo.height			= vkSurfaceHeight;
		frameBufferCreateInfo.layers			= 1;

		ErrorCheck( vkCreateFramebuffer(vkDevice, &frameBufferCreateInfo, nullptr, &vkFrameBuffer[i]) );
	}
}

void VkRenderer::deInitFrameBuffer()
{
	for (uint32_t i = 0; i < vkSwapChainImageCount; i++) {
		vkDestroyFramebuffer(vkDevice, vkFrameBuffer[i], nullptr);
	}
}

void VkRenderer::initSynchronization()
{
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(vkDevice, &fenceCreateInfo, nullptr, &vkSwapChainImageAvailable);
}

void VkRenderer::deInitSynchronization()
{
	vkDestroyFence(vkDevice, vkSwapChainImageAvailable, nullptr);	vkSwapChainImageAvailable = VK_NULL_HANDLE;
}

uint32_t VkRenderer::getVkSurfaceWidth() const
{
	return vkSurfaceWidth;
}

uint32_t VkRenderer::getVkSurfaceHeight() const
{
	return vkSurfaceHeight;
}

void VkRenderer::beginRender()
{
	vkAcquireNextImageKHR(vkDevice, vkSwapChain, UINT64_MAX, VK_NULL_HANDLE, vkSwapChainImageAvailable, &vkActiveSwapChainID);
	vkWaitForFences(vkDevice, 1, &vkSwapChainImageAvailable, VK_TRUE, UINT64_MAX);
	vkResetFences(vkDevice, 1, &vkSwapChainImageAvailable);
	vkQueueWaitIdle(vkQueue);
}

void VkRenderer::endRender(const std::vector<VkSemaphore>& waitSemaphores)
{
	VkResult presentResult = VkResult::VK_RESULT_MAX_ENUM;

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount	= static_cast<uint32_t>(waitSemaphores.size());
	presentInfo.pWaitSemaphores		= waitSemaphores.data();
	presentInfo.swapchainCount		= 1;
	presentInfo.pSwapchains			= &vkSwapChain;
	presentInfo.pImageIndices		= &vkActiveSwapChainID;
	presentInfo.pResults			= &presentResult;

	ErrorCheck(vkQueuePresentKHR(vkQueue, &presentInfo));
}

VkCommandPool VkRenderer::createCommandPool()
{
	VkCommandPool cmdPool = VK_NULL_HANDLE;

	VkCommandPoolCreateInfo cmdPoolCreateInfo{};
	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolCreateInfo.queueFamilyIndex = getVkGraphicsQueueFamilyIndex();
	vkCreateCommandPool(vkDevice, &cmdPoolCreateInfo, nullptr, &cmdPool);

	return cmdPool;
}

VkCommandBuffer VkRenderer::createCommandBuffer(VkCommandPool cmdPool)
{
	VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;

	VkCommandBufferAllocateInfo cmdBufferAllocateInfo{};
	cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocateInfo.commandPool = cmdPool;
	cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocateInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(vkDevice, &cmdBufferAllocateInfo, &cmdBuffer);

	return cmdBuffer;
}

VkSemaphore VkRenderer::createSemaphore()
{
	VkSemaphore semaphore = VK_NULL_HANDLE;

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(vkDevice, &semaphoreCreateInfo, nullptr, &semaphore);

	return semaphore;
}

VkShaderModule VkRenderer::createShaderModule(const std::string& spirvShaderFile)
{
	VkShaderModule shaderModule = VK_NULL_HANDLE;

	std::string spirvShaderSrc = convertFileToString(spirvShaderFile);
	const char* spirvShaderSrcCharPtr = spirvShaderSrc.data();

	VkShaderModuleCreateInfo vertexShaderModuleCreateInfo{};
	vertexShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertexShaderModuleCreateInfo.codeSize = static_cast<uint32_t>(spirvShaderSrc.size());
	vertexShaderModuleCreateInfo.pCode = (uint32_t*)spirvShaderSrcCharPtr;

	vkCreateShaderModule(vkDevice, &vertexShaderModuleCreateInfo, nullptr, &shaderModule);

	return shaderModule;
}

Buffer VkRenderer::createBuffer(VkBufferUsageFlags usageFlags, size_t bufferSize)
{
	Buffer buffer = {VK_NULL_HANDLE, VK_NULL_HANDLE};

	uint32_t queueIndex = getVkGraphicsQueueFamilyIndex();

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = usageFlags;
	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.queueFamilyIndexCount = 1;
	bufferCreateInfo.pQueueFamilyIndices = &queueIndex;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &buffer.vkBuffer);

	VkMemoryRequirements bufferMemReqs{};
	vkGetBufferMemoryRequirements(vkDevice, buffer.vkBuffer, &bufferMemReqs);

	VkMemoryAllocateInfo bufferMemAllocInfo{};
	bufferMemAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferMemAllocInfo.allocationSize = bufferMemReqs.size;
	bufferMemAllocInfo.memoryTypeIndex = FindVkMemoryTypeIndex(getVkPhysicalDeviceMemProperties(), bufferMemReqs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	vkAllocateMemory(vkDevice, &bufferMemAllocInfo, nullptr, &buffer.vkBufferMemory);

	vkBindBufferMemory(vkDevice, buffer.vkBuffer, buffer.vkBufferMemory, 0);

	return buffer;
}

void VkRenderer::destroyBuffer(Buffer buffer)
{
	if(buffer.vkBufferMemory != VK_NULL_HANDLE) 
	{
		vkFreeMemory(vkDevice, buffer.vkBufferMemory, nullptr);
		buffer.vkBufferMemory = VK_NULL_HANDLE;
	}

	if (buffer.vkBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(vkDevice, buffer.vkBuffer, nullptr);
		buffer.vkBuffer = VK_NULL_HANDLE;
	}
}
