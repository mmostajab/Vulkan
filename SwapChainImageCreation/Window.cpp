// To prevent problem with min/max macros
#define NOMINMAX

#include "Window.h"

#include "Helper.h"
#include "Renderer.h"

// STD
#include <assert.h>
#include <limits>
 


Window::Window(std::shared_ptr<Renderer> r, uint32_t x, uint32_t y, std::string name):
	renderer(r),
	surface_size_x(x),
	surface_size_y(y),
	window_name(name)
{
	initWindow();
	initSurface();
	initSwapChain();
	initSwapChainImages();
}

Window::~Window()
{
	deInitSwapChainImages();
	deInitSwapChain();
	deInitSurface();
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

void Window::initSurface()
{
	initOSSurface();

	auto gpu = renderer->getVulkanPhysicalDevice();

	VkBool32 WSI_supported = false;
	CHECK_ERROR( vkGetPhysicalDeviceSurfaceSupportKHR(gpu, renderer->getVulkanGraphicsQueueFamilyIndex(), surface, &WSI_supported) );
	if (!WSI_supported) {
		assert(0 && "WSI not supported.");
		std::exit(-1);
	}

	// Swap chain size should match the surface size.
	CHECK_ERROR( vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_caps) );
	if (surface_caps.currentExtent.width < std::numeric_limits<uint32_t>::max()) {
		surface_size_x = surface_caps.currentExtent.width;
		surface_size_y = surface_caps.currentExtent.height;
	}

	uint32_t surface_format_count = 0;
	CHECK_ERROR( vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &surface_format_count, nullptr) );
	if (surface_format_count == 0) {
		assert(0 && "Surface format is missing.");
		std::exit(-1);
	}
	std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
	CHECK_ERROR( vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &surface_format_count, surface_formats.data()) );
	if (surface_formats[0].format == VK_FORMAT_UNDEFINED) {
		surface_format.format		= VK_FORMAT_B8G8R8A8_UNORM;
		surface_format.colorSpace	= VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	}
	else {
		surface_format = surface_formats[0];
	}
}

void Window::deInitSurface()
{
	vkDestroySurfaceKHR(renderer->getVulkanInstance(), surface, nullptr);
	surface = VK_NULL_HANDLE;
}

void Window::initSwapChain()
{
	if (swapchain_image_count < surface_caps.minImageCount + 1) swapchain_image_count = surface_caps.minImageCount + 1;
	if (surface_caps.maxImageCount > 0)
		if (swapchain_image_count > surface_caps.maxImageCount)		swapchain_image_count = surface_caps.maxImageCount;

	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	uint32_t present_mode_count;
	CHECK_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->getVulkanPhysicalDevice(), surface, &present_mode_count, nullptr));
	std::vector<VkPresentModeKHR> present_modes(present_mode_count);
	CHECK_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->getVulkanPhysicalDevice(), surface, &present_mode_count, present_modes.data()));
	for (auto m : present_modes) {
		if (m == VK_PRESENT_MODE_MAILBOX_KHR) present_mode = m;
	}

	VkSwapchainCreateInfoKHR swapchain_create_info{};
	swapchain_create_info.sType					= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface				= surface;
	swapchain_create_info.minImageCount			= swapchain_image_count;
	swapchain_create_info.imageFormat			= surface_format.format;
	swapchain_create_info.imageColorSpace		= surface_format.colorSpace;
	swapchain_create_info.imageExtent.width		= surface_size_x;
	swapchain_create_info.imageExtent.height	= surface_size_y;
	swapchain_create_info.imageArrayLayers		= 1; // 2 for stereoscopic rendering
	swapchain_create_info.imageUsage			= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_create_info.imageSharingMode		= VK_SHARING_MODE_EXCLUSIVE;
	swapchain_create_info.queueFamilyIndexCount = 0;
	swapchain_create_info.pQueueFamilyIndices   = nullptr;
	swapchain_create_info.preTransform			= VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchain_create_info.compositeAlpha		= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode			= present_mode;
	swapchain_create_info.clipped				= VK_TRUE;
	swapchain_create_info.oldSwapchain			= VK_NULL_HANDLE;
	
	CHECK_ERROR( vkCreateSwapchainKHR(renderer->getVulkanDevice(), &swapchain_create_info, nullptr, &swapchain) );
	CHECK_ERROR( vkGetSwapchainImagesKHR(renderer->getVulkanDevice(), swapchain, &swapchain_image_count, nullptr) );
}

void Window::deInitSwapChain()
{
	vkDestroySwapchainKHR(renderer->getVulkanDevice(), swapchain, nullptr);
}

void Window::initSwapChainImages()
{
	swapchain_images.resize(swapchain_image_count);
	swapchain_imageviews.resize(swapchain_image_count);

	CHECK_ERROR(vkGetSwapchainImagesKHR(renderer->getVulkanDevice(), swapchain, &swapchain_image_count, swapchain_images.data()));

	for (uint32_t i = 0; i < swapchain_image_count; i++) {
		VkImageViewCreateInfo create_info{};
		create_info.sType							= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.image							= swapchain_images[i];
		create_info.viewType						= VK_IMAGE_VIEW_TYPE_2D;
		create_info.format							= surface_format.format;
		create_info.components.r					= VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g					= VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b					= VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a					= VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel	= 0;
		create_info.subresourceRange.levelCount		= 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount		= 1;
		
		CHECK_ERROR( vkCreateImageView(renderer->getVulkanDevice(), &create_info, nullptr, &swapchain_imageviews[i]) );
	}
}

void Window::deInitSwapChainImages()
{
	for (uint32_t i = 0; i < swapchain_image_count; i++) {
		vkDestroyImageView(renderer->getVulkanDevice(), swapchain_imageviews[i], nullptr);
	}
}
