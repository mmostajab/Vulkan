#pragma once

#include <vulkan/vulkan.h>
#include <string>

class ShaderStage
{
public:
	ShaderStage();
	~ShaderStage();

	bool fromHLSLSource (VkDevice device, const char* src, uint32_t len, VkShaderStageFlagBits shaderStageType, const char* entryFunc);
	bool fromGLSLSource (VkDevice device, const char* src, uint32_t len, VkShaderStageFlagBits shaderStageType, const char* entryFunc);
	bool fromSPIRVSource(VkDevice device, const char* src, uint32_t len, VkShaderStageFlagBits shaderStageType, const char* entryFunc);

	bool fromHLSLFile (VkDevice device, const char* path, VkShaderStageFlagBits shaderStageType, const char* entryFunc);
	bool fromGLSLFile (VkDevice device, const char* path, VkShaderStageFlagBits shaderStageType, const char* entryFunc);
	bool fromSPIRVFile(VkDevice device, const char* path, VkShaderStageFlagBits shaderStageType, const char* entryFunc);

	VkShaderModule        getVkShaderModule() const;
	VkShaderStageFlagBits getVkShaderType() const;
	const char*           getEntryFuncName() const;

private:
	void clear(VkDevice device);

	const char* getGLSLangValidatorShaderStage(VkShaderStageFlagBits shaderStage);

	VkShaderModule            m_shaderModule  = VK_NULL_HANDLE;
	VkShaderStageFlagBits     m_shaderType    = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	std::string               m_entryFunction = "";
};