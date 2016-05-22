#pragma once

#include "Platform.h"

#include <string>
#include <memory>
#include <vector>

class Renderer;

class Window {
public:
	Window(std::shared_ptr<Renderer> r, uint32_t x, uint32_t y, std::string name);
	~Window();

	void close();

	// 
	bool update();

private:

	void initWindow();
	void deInitWindow();
	void updateOSWindow();
	void initOSSurface();
	
	void initSurface();
	void deInitSurface();

	void initSwapChain();
	void deInitSwapChain();

	uint32_t					surface_size_x		= 512;
	uint32_t					surface_size_y		= 512;
	std::string					window_name			= "Test Window";
	uint32_t                    swapchain_image_count = 2;
	
	VkSurfaceKHR				surface				= VK_NULL_HANDLE;
	VkSurfaceFormatKHR          surface_format		= {};
	VkSurfaceCapabilitiesKHR	surface_caps		= {};
	VkSwapchainKHR              swapchain			= VK_NULL_HANDLE;
	std::vector<VkImage>        swapchain_images;
	
	bool						windowIsRunning		= false;

	std::shared_ptr<Renderer>	renderer			= nullptr;

#ifdef VK_USE_PLATFORM_WIN32_KHR
	HINSTANCE        win32_instance			= NULL;
	HWND             win32_window			= NULL;
	std::string      win32_class_name;
	static uint64_t  win32_class_id_counter;
#endif
};