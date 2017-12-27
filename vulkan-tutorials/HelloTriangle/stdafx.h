// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// STD
#include <iostream>
#include <stdexcept>
#include <functional>
#include <memory>
#include <stdio.h>
#include <tchar.h>

// GLFW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Vk
#include <vulkan/vulkan.hpp>
#include "VkDeleter.h"

// const expression for const values
struct Rect2D {
	int x, y;
	int width, height;
};
constexpr Rect2D windowRect = { 0, 0, 800, 600 };