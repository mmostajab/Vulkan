#pragma once

// STD
#include <iostream>
#include <string>
#include <assert.h>

#include "Platform.h"

void ErrorCheck(std::string filename, uint32_t line, VkResult result);

#define CHECK_ERROR(result) ErrorCheck(__FILE__, __LINE__, result)
