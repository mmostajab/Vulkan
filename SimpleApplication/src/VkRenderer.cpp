#include "VkRenderer.h"

// STD
#include <iostream>
#include <assert.h>
#include <vector>
#include <sstream>

#ifdef _WIN32
	// Windows
	#include <Windows.h>
#endif

VkRenderer::VkRenderer()
{
	setupDebug();
	initInstance("test");
	initDebug();
	initDevice();
}


VkRenderer::~VkRenderer()
{
	deInitDevice();
	deInitDebug();
	deInitInstance();
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
	instanceCreateInfo.enabledLayerCount       = static_cast<uint32_t>(vkValidationLayerList.size());
	instanceCreateInfo.ppEnabledLayerNames     = vkValidationLayerList.data();
	instanceCreateInfo.pNext                   = &debugReportCallbackCreateInfo;

	auto err = vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance);
	if (VK_SUCCESS != err) {
		assert(1 && "Vulkan Error: Create Instance failed.");
		std::exit(-1);
	}
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

void VkRenderer::initDevice()
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
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(vkGPU, &physicalDeviceProperties);
	std::cout << "Device Name = " << physicalDeviceProperties.deviceName << std::endl;
	std::cout << "Device Type = " << getDeviceTypeString(physicalDeviceProperties.deviceType) << std::endl;
	std::cout << "Device API version = " << getAPIVersionString(physicalDeviceProperties.apiVersion) << std::endl;	
	
	// ========================================
	// Physical Device Queue Family extraction
	// ========================================
	uint32_t numPhysicalDeviceQueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(vkGPU, &numPhysicalDeviceQueueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> physicalDeviceQueueFamilyPropertiesList(numPhysicalDeviceQueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(vkGPU, &numPhysicalDeviceQueueFamilyCount, physicalDeviceQueueFamilyPropertiesList.data());
	bool found = false;
	for (uint32_t i = 0; i < physicalDeviceQueueFamilyPropertiesList.size(); i++) {
		if (physicalDeviceQueueFamilyPropertiesList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			found = true;
			vkGraphicsFamilyIndex = i;
		}
	}

	if (!found) {
		assert(1 && "Vulkan ERROR: Queue family supporting graphics not found.");
		std::exit(-1);
	}

	// ==================================================
	// Validation Layers extraction: Instance
	// ==================================================
	uint32_t instanceValidationLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&instanceValidationLayerCount, nullptr);
	std::vector<VkLayerProperties> instanceValidationLayerPropertyList(instanceValidationLayerCount);
	vkEnumerateInstanceLayerProperties(&instanceValidationLayerCount, instanceValidationLayerPropertyList.data());
	std::cout << "Instance Layers " << std::endl;
	for (auto& i : instanceValidationLayerPropertyList) {
		std::cout << "\t" << i.layerName << "\t | " << i.description << std::endl;
	}
	std::cout << std::endl;

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
	deviceCreateInfo.enabledExtensionCount		= static_cast<uint32_t>(vkExtensionsList.size());
	deviceCreateInfo.ppEnabledExtensionNames	= vkExtensionsList.data();
	deviceCreateInfo.enabledLayerCount			= static_cast<uint32_t>(vkValidationLayerList.size());
	deviceCreateInfo.ppEnabledLayerNames		= vkValidationLayerList.data();

	vkCreateDevice(vkGPU, &deviceCreateInfo, nullptr, &vkDevice);
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

void VkRenderer::setupDebug()
{
	//vkValidationLayerList.push_back("VK_LAYER_LUNARG_api_dump");
	vkValidationLayerList.push_back("VK_LAYER_LUNARG_core_validation");
	//vkValidationLayerList.push_back("VK_LAYER_LUNARG_monitor");
	vkValidationLayerList.push_back("VK_LAYER_GOOGLE_threading");
	vkValidationLayerList.push_back("VK_LAYER_LUNARG_swapchain");
	vkValidationLayerList.push_back("VK_LAYER_LUNARG_image");
	vkValidationLayerList.push_back("VK_LAYER_LUNARG_object_tracker");
	vkValidationLayerList.push_back("VK_LAYER_GOOGLE_unique_objects");
	vkValidationLayerList.push_back("VK_LAYER_LUNARG_parameter_validation");
	//vkValidationLayerList.push_back("VK_LAYER_LUNARG_screenshot");
	//vkValidationLayerList.push_back("VK_LAYER_LUNARG_vktrace");
	//vkValidationLayerList.push_back("VK_LAYER_RENDERDOC_Capture");
	//vkValidationLayerList.push_back("VK_LAYER_LUNARG_standard_validation");

	vkExtensionsList.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugReportCallbackCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)VulkanDebugCallBackFunc;
	debugReportCallbackCreateInfo.flags =
		VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_DEBUG_BIT_EXT;
}

PFN_vkCreateDebugReportCallbackEXT	fvkCreateDebugReportCallBackExt = nullptr;
PFN_vkDestroyDebugReportCallbackEXT	fvkDestroyDebugReportCallBackExt = nullptr;

void VkRenderer::initDebug()
{
	fvkCreateDebugReportCallBackExt  = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");
	fvkDestroyDebugReportCallBackExt = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
	if (fvkCreateDebugReportCallBackExt == nullptr || fvkDestroyDebugReportCallBackExt == nullptr) {
		assert(1 && "Vulkan ERROR: cannot fetch debug function pointers.");
		exit(-1);
	}	

	fvkCreateDebugReportCallBackExt(vkInstance, &debugReportCallbackCreateInfo, nullptr, &debugReport);
}

void VkRenderer::deInitDebug()
{
	fvkDestroyDebugReportCallBackExt(vkInstance, debugReport, nullptr);
	debugReport = nullptr;
}
