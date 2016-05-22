#pragma once

#include "Platform.h"

#include <string>

class Window {
public:
	Window(uint32_t x, uint32_t y, std::string name);
	~Window();

	void close();

	// 
	bool update();

private:

	void initWindow();
	void deInitWindow();
	void updateOSWindow();
	void initOSSurface();

	uint32_t    surface_size_x    = 512;
	uint32_t    surface_size_y    = 512;
	std::string window_name;
	bool        windowIsRunning   = false;

#ifdef VK_USE_PLATFORM_WIN32_KHR
	HINSTANCE        win32_instance			= NULL;
	HWND             win32_window			= NULL;
	std::string      win32_class_name;
	static uint64_t  win32_class_id_counter;
#endif
};