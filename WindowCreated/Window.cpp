#include "Window.h"

Window::Window(uint32_t x, uint32_t y, std::string name):
	surface_size_x(x),
	surface_size_y(y),
	window_name(name)
{
	initWindow();
}

Window::~Window()
{
	deInitWindow();
}

void Window::close()
{
	windowIsRunning = false;
}

bool Window::update()
{
	updateOSWindow();
	return windowIsRunning;
}
