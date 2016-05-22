#include "Window.h"

#include "BUILD_OPTIONS.h"
#include "Platform.h"

// STD
#include <assert.h>

#ifdef VK_USE_PLATFORM_WIN32_KHR

// Microsoft Windows specific versions of window functions
LRESULT CALLBACK WindowsEventHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Window * window = reinterpret_cast<Window*>(
		GetWindowLongPtrW(hWnd, GWLP_USERDATA));

	switch (uMsg) {
	case WM_CLOSE:
		window->close();
		return 0;
	case WM_SIZE:
		// we get here if the window has changed size, we should rebuild most
		// of our window resources before rendering to this window again.
		// ( no need for this because our window sizing by hand is disabled )
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

uint64_t	Window::win32_class_id_counter = 0;

void Window::initWindow()
{
	WNDCLASSEXA win_class{};
	assert(surface_size_x > 0);
	assert(surface_size_y > 0);

	win32_instance = GetModuleHandle(nullptr);
	win32_class_name = window_name + "_" + std::to_string(win32_class_id_counter);
	win32_class_id_counter++;

	// Initialize the window class structure:
	win_class.cbSize = sizeof(WNDCLASSEXA);
	win_class.style = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = WindowsEventHandler;
	win_class.cbClsExtra = 0;
	win_class.cbWndExtra = 0;
	win_class.hInstance = win32_instance; // hInstance
	win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	win_class.lpszMenuName = NULL;
	win_class.lpszClassName = win32_class_name.c_str();
	win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
	// Register window class:
	if (!RegisterClassExA(&win_class)) {
		// It didn't work, so try to give a useful error:
		assert(0 && "Cannot create a window in which to draw!\n");
		fflush(stdout);
		std::exit(-1);
	}

	DWORD ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	// Create window with the registered class:
	RECT wr = { 0, 0, LONG(surface_size_x), LONG(surface_size_y) };
	AdjustWindowRectEx(&wr, style, FALSE, ex_style);
	win32_window = CreateWindowExA(0,
		win32_class_name.c_str(),		// class name
		window_name.c_str(),			// app name
		style,							// window style
		CW_USEDEFAULT, CW_USEDEFAULT,	// x/y coords
		wr.right - wr.left,				// width
		wr.bottom - wr.top,				// height
		NULL,							// handle to parent
		NULL,							// handle to menu
		win32_instance,				// hInstance
		NULL);							// no extra parameters
	if (!win32_window) {
		// It didn't work, so try to give a useful error:
		assert(1 && "Cannot create a window in which to draw!\n");
		fflush(stdout);
		std::exit(-1);
	}
	SetWindowLongPtr(win32_window, GWLP_USERDATA, (LONG_PTR)this);

	ShowWindow(win32_window, SW_SHOW);
	SetForegroundWindow(win32_window);
	SetFocus(win32_window);

	// Set the window in running state
	windowIsRunning = true;
}


void Window::deInitWindow()
{
	DestroyWindow(win32_window);
	UnregisterClassA(win32_class_name.c_str(), win32_instance);
}

void Window::updateOSWindow()
{
	MSG msg;
	if (PeekMessageA(&msg, win32_window, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

void Window::initOSSurface()
{
}

#endif