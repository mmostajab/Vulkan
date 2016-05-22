#include "Renderer.h"

// STD
#include <cstdlib>
#include <assert.h>
#include <iostream>
#include <sstream>

#include "Helper.h"
#include "BUILD_OPTIONS.h"
#include "Platform.h"
#include "Window.h"

Renderer::Renderer()
{
	setupLayersAndExtensions();
	setupDebug();
	initInstance();
	initDebug();
	initDevice();
}


Renderer::~Renderer()
{
	deInitDevice();
	deInitDebug();
	deInitInstance();
}

void Renderer::createWindow(uint32_t x, uint32_t y, std::string name)
{
	window = std::make_unique<Window>( shared_from_this(), x, y, name );
}

bool Renderer::run()
{
	if(nullptr != window)
		return window->update();
	return false;
}

const VkInstance Renderer::getVulkanInstance() const
{
	return vkInstance;
}

const VkPhysicalDevice Renderer::getVulkanPhysicalDevice() const
{
	return gpu;
}

const VkPhysicalDeviceProperties& Renderer::getVulkanPhysicalDeviceProperties() const
{
	return gpu_properties;
}

const VkDevice Renderer::getVulkanDevice() const
{
	return vkDevice;
}

const uint32_t Renderer::getVulkanGraphicsQueueFamilyIndex() const
{
	return graphicsFamilyIdx;
}

void Renderer::setupLayersAndExtensions()
{
	// Surface support
	//instance_exts.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
	instance_exts.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	instance_exts.push_back(PLATFORM_SURFACE_EXTENSION_NAME);

	// Swap chain support
	device_exts.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

void Renderer::initInstance()
{
	VkApplicationInfo application_info{};
	application_info.sType					= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.apiVersion				= VK_MAKE_VERSION(1, 0, 11);
	application_info.applicationVersion		= VK_MAKE_VERSION(0, 1, 0);
	application_info.pApplicationName		= "Vulkan Tutorial Simple";

	VkInstanceCreateInfo instance_create_info{};
	instance_create_info.sType						= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;	// type of the structure
	instance_create_info.pApplicationInfo			= &application_info;
	instance_create_info.enabledLayerCount			= static_cast<uint32_t>(instance_layers.size());
	instance_create_info.ppEnabledLayerNames		= instance_layers.data();
	instance_create_info.enabledExtensionCount		= static_cast<uint32_t>(instance_exts.size());
	instance_create_info.ppEnabledExtensionNames	= instance_exts.data();
	instance_create_info.pNext						= &debug_callback_create_info;

	// err = 0 = > Success
	CHECK_ERROR( vkCreateInstance(&instance_create_info, nullptr, &vkInstance) );
}

void Renderer::deInitInstance()
{
	vkDestroyInstance(vkInstance, nullptr);
	vkInstance = nullptr;
}

void Renderer::initDevice()
{
	// Device is equivalence to the opengl context.
	uint32_t gpu_count = 0;
	vkEnumeratePhysicalDevices(vkInstance, &gpu_count, nullptr);
	std::vector<VkPhysicalDevice> gpu_list(gpu_count);
	vkEnumeratePhysicalDevices(vkInstance, &gpu_count, gpu_list.data());
	gpu = gpu_list[0];

	vkGetPhysicalDeviceProperties(gpu, &gpu_properties);

	std::cout << "===========================================================================\n";
	std::cout << "== GPU: " << gpu_properties.deviceName << std::endl;
	std::cout << "===========================================================================\n";

	// Physical device queue families
	uint32_t family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, nullptr);
	std::vector<VkQueueFamilyProperties> family_propery_list(family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, family_propery_list.data());

	bool found = false;
	for (uint32_t i = 0; i < family_count; i++) {
		if (family_propery_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			found = true;
			graphicsFamilyIdx = i;
		}
	}

	if (!found) {
		assert(0 && "VULKAN ERROR: Queue family supporting graphics not found.");
		std::exit(-1);
	}

	uint32_t layer_count = 0;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	std::vector<VkLayerProperties> layer_property_list(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, layer_property_list.data());
	std::cout << "===========================================================================\n";
	std::cout << "== Instance layer propery list: \n";
	std::cout << "===========================================================================\n";
	for (auto& layer_property : layer_property_list) {
		std::cout << layer_property.layerName << "\t\t\t" << layer_property.description << std::endl;
	}
	std::cout << "===========================================================================\n\n\n";

	uint32_t device_layer_count = 0;
	vkEnumerateDeviceLayerProperties(gpu, &device_layer_count, nullptr);
	std::vector<VkLayerProperties> device_layer_property_list(layer_count);
	vkEnumerateDeviceLayerProperties(gpu, &device_layer_count, device_layer_property_list.data());
	std::cout << "===========================================================================\n";
	std::cout << "== Device layer propery list: \n";
	std::cout << "===========================================================================\n";
	for (auto& layer_property : device_layer_property_list) {
		std::cout << layer_property.layerName << "\t\t\t" << layer_property.description << std::endl;
	}
	std::cout << "===========================================================================\n\n\n";

	float queue_priorities[] = { 1.0 };

	VkDeviceQueueCreateInfo device_queue_create_info{};
	device_queue_create_info.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_create_info.queueFamilyIndex	= graphicsFamilyIdx;
	device_queue_create_info.queueCount			= 1;
	device_queue_create_info.pQueuePriorities	= queue_priorities;

	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount		= 1;
	device_create_info.pQueueCreateInfos		= &device_queue_create_info;
	device_create_info.enabledLayerCount		= static_cast<uint32_t>(device_layers.size());
	device_create_info.ppEnabledLayerNames		= device_layers.data();
	device_create_info.enabledExtensionCount	= static_cast<uint32_t>(device_exts.size());
	device_create_info.ppEnabledExtensionNames	= device_exts.data();

	CHECK_ERROR( vkCreateDevice(gpu, &device_create_info, nullptr, &vkDevice) );

	vkGetDeviceQueue(vkDevice, graphicsFamilyIdx, 0, &vkQueue);
}

void Renderer::deInitDevice()
{
	vkDestroyDevice(vkDevice, nullptr);
	vkDevice = nullptr;
}

#if BUILD_ENABLE_VULKAN_DEBUG

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
	VkDebugReportFlagsEXT		flags,
	VkDebugReportObjectTypeEXT  obj_type,
	uint64_t                    src_object,
	size_t                      location,
	int32_t                     msg_code,
	const char*                 layer_prefix,
	const char*                 msg,
	void*                       user_data) {

	std::ostringstream msg_stream;

	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		msg_stream << "[INFO]: \t";
	} else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		msg_stream << "[WARNING]: \t";
	}
	else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		msg_stream << "[PERF_WARNING]: \t";
	}
	else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		msg_stream << "[ERROR]: \t";
	}
	else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		msg_stream << "[DEBUG]: \t";
	}

	msg_stream << " @" << layer_prefix << "\t";

	msg_stream << msg << std::endl;

	std::cout << msg_stream.str();

#ifdef WIN32
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		MessageBoxA(NULL, msg_stream.str().c_str(), "Vulkan Error!", 0);
	}
#endif

	return false;
}

void Renderer::setupDebug()
{
	debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debug_callback_create_info.pfnCallback = VulkanDebugCallback;
	debug_callback_create_info.flags =
//		VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
//		VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		0;

	instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	instance_layers.push_back("VK_LAYER_GOOGLE_threading");
	//instance_layers.push_back("VK_LAYER_LUNARG_draw_state");
	instance_layers.push_back("VK_LAYER_LUNARG_image");
	//instance_layers.push_back("VK_LAYER_LUNARG_mem_tracker");
	instance_layers.push_back("VK_LAYER_LUNARG_object_tracker");
	instance_layers.push_back("VK_LAYER_LUNARG_parameter_validation");

	instance_exts.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	
	//device_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	device_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	device_layers.push_back("VK_LAYER_GOOGLE_threading");
	//device_layers.push_back("VK_LAYER_LUNARG_draw_state");
	device_layers.push_back("VK_LAYER_LUNARG_image");
	//device_layers.push_back("VK_LAYER_LUNARG_mem_tracker");
	device_layers.push_back("VK_LAYER_LUNARG_object_tracker");
	device_layers.push_back("VK_LAYER_LUNARG_parameter_validation");
}

PFN_vkCreateDebugReportCallbackEXT  fvkCreateDebugReportCallbackEXT  = nullptr;
PFN_vkDestroyDebugReportCallbackEXT fvkDestoryDebugReportCallbackEXT = nullptr;

void Renderer::initDebug()
{
	fvkCreateDebugReportCallbackEXT  = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");
	fvkDestoryDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
	if (nullptr == fvkCreateDebugReportCallbackEXT || nullptr == fvkDestoryDebugReportCallbackEXT) {
		assert(0 && "Vulkan Error: Can't fetch debug function pointers");
		std::exit(-1);
	}

	fvkCreateDebugReportCallbackEXT(vkInstance, &debug_callback_create_info, nullptr, &debug_report);
}

void Renderer::deInitDebug()
{
	fvkDestoryDebugReportCallbackEXT(vkInstance, debug_report, nullptr);
	debug_report = nullptr;
}

#else

void Renderer::setupDebug() {}
void Renderer::initDebug()  {}
void Renderer::deInitDebug(){}

#endif