#include "Window.h"

#include "Renderer.h"

Window::Window(std::shared_ptr<Renderer> r, uint32_t x, uint32_t y, std::string name):
	renderer(r),
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
