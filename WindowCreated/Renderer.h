#pragma once

#include <vector>
#include <memory>

#include "Platform.h"
#include "Window.h"

class Renderer
{
public:
	Renderer();
	~Renderer();

	void createWindow(uint32_t x, uint32_t y, std::string name);
	

	bool    run();

//private:
	void initInstance();
	void deInitInstance();

	void initDevice();
	void deInitDevice();

	void setupDebug();
	void initDebug();
	void deInitDebug();

	VkInstance					vkInstance			= nullptr;
	VkPhysicalDevice			gpu					= nullptr;
	VkPhysicalDeviceProperties	gpu_properties;
	uint32_t					graphicsFamilyIdx	= 0;
	VkDevice					vkDevice			= nullptr;
	VkQueue                     vkQueue				= nullptr;

	std::unique_ptr<Window>     window              = nullptr;

	std::vector<const char*>    instance_layers;
	std::vector<const char*>    instance_exts;
	std::vector<const char*>    device_layers;
	std::vector<const char*>    device_exts;

	VkDebugReportCallbackEXT			debug_report;
	VkDebugReportCallbackCreateInfoEXT	debug_callback_create_info{};
};

