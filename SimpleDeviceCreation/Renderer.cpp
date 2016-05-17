#include "Renderer.h"
#include <cstdlib>
#include <assert.h>
#include <vector>
#include <iostream>

Renderer::Renderer()
{
	initInstance();
	initDevice();
}


Renderer::~Renderer()
{
	deInitDevice();
	deInitInstance();
}

void Renderer::initInstance()
{
	VkApplicationInfo application_info{};
	application_info.sType					= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.apiVersion				= VK_MAKE_VERSION(1, 0, 11);
	application_info.pApplicationName		= "Vulkan Tutorial Simple";

	VkInstanceCreateInfo instance_create_info{};
	instance_create_info.sType				= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;	// type of the structure
	instance_create_info.pApplicationInfo	= &application_info;

	// err = 0 = > Success
	VkResult err = vkCreateInstance(&instance_create_info, nullptr, &vkInstance);
	if (VK_SUCCESS != err) {
		assert(1 && "Vulkan Error: Create Instance failed.");
		std::exit(-1);
	}
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

	std::cout << "GPU: " << gpu_properties.deviceName << std::endl;

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
		assert(1 && "VULKAN ERROR: Queue family supporting graphics not found.");
		std::exit(-1);
	}

	float queue_priorities[] = { 1.0 };

	VkDeviceQueueCreateInfo device_queue_create_info{};
	device_queue_create_info.sType				= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_queue_create_info.queueFamilyIndex	= graphicsFamilyIdx;
	device_queue_create_info.queueCount			= 1;
	device_queue_create_info.pQueuePriorities	= queue_priorities;

	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType				= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pQueueCreateInfos	= &device_queue_create_info;


	vkCreateDevice(gpu, &device_create_info, nullptr, &vkDevice);
}

void Renderer::deInitDevice()
{
	vkDestroyDevice(vkDevice, nullptr);
	vkDevice = nullptr;
}
