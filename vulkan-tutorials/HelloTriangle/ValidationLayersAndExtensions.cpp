#include "stdafx.h"
#include "HelloTriangleApp.h"
#include "VkHelper.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <windows.h>

std::string GetAPIVersionString(uint32_t apiVersion)
{
	std::ostringstream apiVersionString;
	apiVersionString << VK_VERSION_MAJOR(apiVersion) << "." << VK_VERSION_MINOR(apiVersion) << "." << VK_VERSION_PATCH(apiVersion);
	return apiVersionString.str();
}

bool HelloTriangleApp::checkInstanceExtensionsSupport(const std::vector<const char*>& requiredExtensions)
{
	bool allExtensionsAvailable = true;

	//
	// Get List of available extensions
	//
	{
		uint32_t extensionCount = 0;
		std::vector<VkExtensionProperties> availableExtensions;
		std::vector<bool> requestedFlags;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		availableExtensions.resize(extensionCount);
		requestedFlags.resize(extensionCount, false);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

		for (const auto& reqExt: requiredExtensions)
		{
			bool found = false;
			for (size_t i = 0; i < availableExtensions.size(); ++i)
			{
				const auto& availableExt = availableExtensions[i];
				if (strcmp(availableExt.extensionName, reqExt) == 0)
				{
					requestedFlags[i] = true;
					found = true;
					break;
				}
			}
			
			if (!found)
			{
				allExtensionsAvailable = false;
				std::clog << "Warning (Instance) : " << reqExt << " extension is not available!\n";
			}
		}

		std::clog << "Available extensions = \n";
		for (size_t i = 0; i < availableExtensions.size(); ++i)
		{
			auto& ext = availableExtensions[i];
			std::clog << "\t" << std::left << std::setw(40) << ext.extensionName << "(" << ext.specVersion << ")"
				<< std::setw(2) << "\t" << (requestedFlags[i] ? "[X]" : "[ ]") << "\n";
		}
	}

	return allExtensionsAvailable;
}

bool HelloTriangleApp::checkInstanceValidationLayersSupport(const std::vector<const char*>& requestedValidationLayers)
{
	bool allValidLayersAvailable = true;

	//
	// Get List of available layers
	//
	if(enableValidationLayers)
	{
		uint32_t layerCount = 0;
		std::vector<VkLayerProperties> availableLayers;
		std::vector<bool> requestedFlags;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		availableLayers.resize(layerCount);
		requestedFlags.resize(layerCount, false);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const auto& reqLayer : requestedValidationLayers)
		{
			bool found = false;
			for (size_t i = 0; i < availableLayers.size(); ++i)
			{
				const auto& availableLayer = availableLayers[i];
				if (strcmp(availableLayer.layerName, reqLayer) == 0)
				{
					requestedFlags[i] = true;
					found = true;
					break;
				}
			}

			if (!found)
			{
				allValidLayersAvailable = false;
				std::clog << "Warning (Instance) : " << reqLayer << " layer is not available!\n";
			}
		}

		// output available validation layers
		std::clog << "Available layers = \n";
		for (size_t i = 0; i < availableLayers.size(); ++i)
		{
			auto& layer = availableLayers[i];
			std::clog << "\t" << std::left << std::setw(40) << layer.layerName << std::setw(40) << layer.description 
				<< "\t(" << GetAPIVersionString(layer.specVersion) << ", " << layer.implementationVersion << ")" 
				<< "\t" << (requestedFlags[i] ? "[X]" : "[ ]") << "\n";
		}
	}

	return allValidLayersAvailable;	
}

bool HelloTriangleApp::checkDeviceExtensionsSupport(VkPhysicalDevice physicalDevice, const std::vector<const char*>& requiredExtensions)
{
	bool allExtensionsAvailable = true;

	//
	// Get List of available extensions
	//
	{
		uint32_t extensionCount = 0;
		std::vector<VkExtensionProperties> availableExtensions;
		std::vector<bool> requestedFlags;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
		availableExtensions.resize(extensionCount);
		requestedFlags.resize(extensionCount, false);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		for (const auto& reqExt : requiredExtensions)
		{
			bool found = false;
			for (size_t i = 0; i < availableExtensions.size(); ++i)
			{
				const auto& availableExt = availableExtensions[i];
				if (strcmp(availableExt.extensionName, reqExt) == 0)
				{
					requestedFlags[i] = true;
					found = true;
					break;
				}
			}

			if (!found)
			{
				allExtensionsAvailable = false;
				std::clog << "Warning (Device): " << reqExt << " extension is not available!\n";
			}
		}

		std::clog << "Available device extensions = \n";
		for (size_t i = 0; i < availableExtensions.size(); ++i)
		{
			auto& ext = availableExtensions[i];
			std::clog << "\t" << std::left << std::setw(40) << ext.extensionName << "(" << ext.specVersion << ")"
				<< std::setw(2) << "\t" << (requestedFlags[i] ? "[X]" : "[ ]") << "\n";
		}
	}

	return allExtensionsAvailable;
}

bool HelloTriangleApp::checkDeviceValidationLayersSupport(VkPhysicalDevice physicalDevice, const std::vector<const char*>& requestedValidationLayers)
{
	bool allValidLayersAvailable = true;

	//
	// Get List of available layers
	//
	if (enableValidationLayers)
	{
		uint32_t layerCount = 0;
		std::vector<VkLayerProperties> availableLayers;
		std::vector<bool> requestedFlags;
		vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, nullptr);
		availableLayers.resize(layerCount);
		requestedFlags.resize(layerCount, false);
		vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, availableLayers.data());

		for (const auto& reqLayer : requestedValidationLayers)
		{
			bool found = false;
			for (size_t i = 0; i < availableLayers.size(); ++i)
			{
				const auto& availableLayer = availableLayers[i];
				if (strcmp(availableLayer.layerName, reqLayer) == 0)
				{
					requestedFlags[i] = true;
					found = true;
					break;
				}
			}

			if (!found)
			{
				allValidLayersAvailable = false;
				std::clog << "Warning (Device) : " << reqLayer << " layer is not available!\n";
			}
		}

		// output available validation layers
		std::clog << "Available device layers = \n";
		for (size_t i = 0; i < availableLayers.size(); ++i)
		{
			auto& layer = availableLayers[i];
			std::clog << "\t" << std::left << std::setw(40) << layer.layerName << std::setw(40) << layer.description
				<< "\t(" << GetAPIVersionString(layer.specVersion) << ", " << layer.implementationVersion << ")"
				<< "\t" << (requestedFlags[i] ? "[X]" : "[ ]") << "\n";
		}
	}

	return allValidLayersAvailable;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallBack(
	VkDebugReportFlagBitsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPerfix,
	const char* msg,
	void* userData
)
{
	return static_cast<HelloTriangleApp*>(userData)->debugCallBack(flags, objType, obj, location, code, layerPerfix, msg);
}

bool HelloTriangleApp::debugCallBack(
	VkDebugReportFlagBitsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location,
	int32_t code, const char* layerPerfix, const char* msg
)
{
	std::cerr << "validation layer: " << msg << std::endl;
	return VK_FALSE;
}

void HelloTriangleApp::setupDebugCallback(VkAllocationCallbacks* pAllocator)
{
	VkDebugReportCallbackCreateInfoEXT dbgCreateInfo{};
	dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	dbgCreateInfo.pNext = nullptr;
	dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)DebugCallBack;
	dbgCreateInfo.pUserData = this;

	auto CreateDbgReportCallBack = [&]() -> VkResult {
		auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr)
		{
			return func(instance, &dbgCreateInfo, pAllocator, dbgReportcallback.replace());
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	};

	if (CreateDbgReportCallBack() != VK_SUCCESS)
	{
		std::runtime_error("failed to set up debug callback!");
	}
	else
	{
		std::clog << "Info: Debug report call back registered sucessfully.\n";
	}
}

void DestroyDbgReportCallBack(VkInstance instance, VkDebugReportCallbackEXT callback, VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
};

bool HelloTriangleApp::isDeviceSuitable(VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceFeatures deviceFeatures;
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

	auto indices = findQueueFamilies(physicalDevice);

	const bool extensionSupported = true;
	bool swapChainAdequate = false;
	if (extensionSupported)
	{
		SwapChainSupportDeatils swapChainSupportDetails = querySwapChainSupportDetails(physicalDevice);
		swapChainAdequate = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty();
	}
	
	return
		deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		deviceFeatures.geometryShader &&
		indices.isComplete() &&
		(!extensionSupported || swapChainAdequate);
}

QueueFamilyIndices HelloTriangleApp::findQueueFamilies(VkPhysicalDevice physicalDevice)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	queueFamilyProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

	// find a appropriate queue
	for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
	{
		auto& q = queueFamilyProperties[i];
		if (q.queueCount > 0 && (q.queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
		indices.presentFamily = presentSupport ? i : -1;

		if (indices.isComplete()) 
			break;
	}

	return indices;
}

VkFormat HelloTriangleApp::findDepthFormat()
{
	return 
		VkHelper::findSupportedFormat(
			physicalDevice,
			{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
}