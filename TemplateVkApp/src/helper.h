#ifndef __HELPER_H__
#define __HELPER_H__

// STD
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

// GLM
#include "glm/glm.hpp"

// Vulkan
#include <vulkan/vulkan.h>

// read the file content and generate a string from it.
std::string convertFileToString(const std::string& filename);

// Checks the error result from a vulkan call.
void ErrorCheck(VkResult result);

// 
uint32_t FindVkMemoryTypeIndex(
	const VkPhysicalDeviceMemoryProperties& gpuMemProperties, 
	const VkMemoryRequirements& memRequirements,
	const VkMemoryPropertyFlags& memProperties);


bool executeCommand(char* cmd, const char* directory);
bool deleteFile(const char* path);
#endif