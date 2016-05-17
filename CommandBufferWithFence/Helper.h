#pragma once

// STD
#include <iostream>
#include <string>
#include <assert.h>

// Vulkan
#include <vulkan\vulkan.h>

void ErrorCheck(std::string filename, uint32_t line, VkResult result);

#define CHECK_ERROR(result) ErrorCheck(__FILE__, __LINE__, result)
