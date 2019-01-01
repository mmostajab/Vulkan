#include "helper.h"

#if defined(_WIN32)
	#include <windows.h>
#else

#endif

std::string convertFileToString(const std::string& filename) {
	std::ifstream ifile(filename, std::ios::binary);
	if (!ifile) {
		return std::string("");
	}

	return std::string(std::istreambuf_iterator<char>(ifile), (std::istreambuf_iterator<char>()));

}

void ErrorCheck(VkResult result)
{
	if (result < 0) {
		switch (result) {
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			std::cout << "VK_ERROR_INITIALIZATION_FAILED" << std::endl;
			break;
		case VK_ERROR_DEVICE_LOST:
			std::cout << "VK_ERROR_DEVICE_LOST" << std::endl;
			break;
		case VK_ERROR_MEMORY_MAP_FAILED:
			std::cout << "VK_ERROR_MEMORY_MAP_FAILED" << std::endl;
			break;
		case VK_ERROR_LAYER_NOT_PRESENT:
			std::cout << "VK_ERROR_LAYER_NOT_PRESENT" << std::endl;
			break;
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			std::cout << "VK_ERROR_EXTENSION_NOT_PRESENT" << std::endl;
			break;
		case VK_ERROR_FEATURE_NOT_PRESENT:
			std::cout << "VK_ERROR_FEATURE_NOT_PRESENT" << std::endl;
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			std::cout << "VK_ERROR_INCOMPATIBLE_DRIVER" << std::endl;
			break;
		case VK_ERROR_TOO_MANY_OBJECTS:
			std::cout << "VK_ERROR_TOO_MANY_OBJECTS" << std::endl;
			break;
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			std::cout << "VK_ERROR_FORMAT_NOT_SUPPORTED" << std::endl;
			break;
		case VK_ERROR_SURFACE_LOST_KHR:
			std::cout << "VK_ERROR_SURFACE_LOST_KHR" << std::endl;
			break;
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			std::cout << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" << std::endl;
			break;
		case VK_SUBOPTIMAL_KHR:
			std::cout << "VK_SUBOPTIMAL_KHR" << std::endl;
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
			break;
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			std::cout << "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR" << std::endl;
			break;
		case VK_ERROR_VALIDATION_FAILED_EXT:
			std::cout << "VK_ERROR_VALIDATION_FAILED_EXT" << std::endl;
			break;
		default:
			break;
		}
		assert(0 && "Vulkan runtime error.");
	}
}

uint32_t FindVkMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties & gpuMemProperties, const VkMemoryRequirements & memRequirements, const VkMemoryPropertyFlags & memProperties){
	uint32_t	vkMemTypeIndex = UINT32_MAX;
	for (uint32_t i = 0; i < gpuMemProperties.memoryTypeCount; i++) {
		if (memRequirements.memoryTypeBits & (1 << i)) {
			if ((gpuMemProperties.memoryTypes[i].propertyFlags & memProperties) == memProperties) {
				vkMemTypeIndex = i;
				break;
			}
		}
	}

	assert(vkMemTypeIndex != UINT32_MAX);
	return vkMemTypeIndex;
}

bool executeCommand(char* cmd, const char* directory)
{
#if defined(_WIN32)

	STARTUPINFO info = { sizeof(info) };
	PROCESS_INFORMATION processInfo;
	if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, directory, &info, &processInfo))
	{
		WaitForSingleObject(processInfo.hProcess, INFINITE);
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
		return true;
	}

#else

#error "not implemented";

#endif

	return false;
}

bool deleteFile(const char* path)
{
#if defined(_WIN32)
	return DeleteFileA(path);
#endif
}