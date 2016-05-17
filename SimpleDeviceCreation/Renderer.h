#pragma once

#include <vulkan\vulkan.h>

class Renderer
{
public:
	Renderer();
	~Renderer();

private:
	void initInstance();
	void deInitInstance();

	void initDevice();
	void deInitDevice();

	VkInstance					vkInstance			= nullptr;
	VkPhysicalDevice			gpu					= nullptr;
	VkPhysicalDeviceProperties	gpu_properties;
	uint32_t					graphicsFamilyIdx	= 0;
	VkDevice					vkDevice			= nullptr;
};

