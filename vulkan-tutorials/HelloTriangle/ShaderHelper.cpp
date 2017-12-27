#pragma once

#include "stdafx.h"
#include "ShaderHelper.h"

#include <fstream>

std::vector<char> ShaderHelper::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.good())
	{
		std::runtime_error("Failed to open file " + filename);
		return {};
	}

	size_t fileSize = file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

void ShaderHelper::createShaderModule(VkDevice vkDevice, const std::vector<char>& shaderCode, VkDeleter<VkShaderModule>& shaderModule)
{
	std::vector<uint32_t> shaderCodeAligned((shaderCode.size() + 3) / sizeof(uint32_t));
	memcpy(shaderCodeAligned.data(), shaderCode.data(), shaderCode.size());
	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.pNext = VK_NULL_HANDLE;
	shaderModuleCreateInfo.flags = 0;
	shaderModuleCreateInfo.pCode = shaderCodeAligned.data();
	shaderModuleCreateInfo.codeSize = shaderCode.size();
	
	if(vkCreateShaderModule(vkDevice, &shaderModuleCreateInfo, nullptr, shaderModule.replace()) != VK_SUCCESS)
		std::runtime_error("Cannot create the shader module.");
}
