#pragma once

#include <vector>
#include <string>

#include "stdafx.h"

class ShaderHelper
{
public:
	static std::vector<char> readFile(const std::string& filename);
	static void createShaderModule(VkDevice vkDevice, const std::vector<char>& shaderCode, VkDeleter<VkShaderModule>& shaderModule);
};