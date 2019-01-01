#include "Shader.h"

#include "helper.h"

ShaderStage::ShaderStage()
{
}

ShaderStage::~ShaderStage()
{
}

bool ShaderStage::fromHLSLSource(VkDevice device, const char * src, uint32_t len, VkShaderStageFlagBits shaderStageType, const char * entryFunc)
{
	return false;
}

bool ShaderStage::fromGLSLSource(VkDevice device, const char * src, uint32_t len, VkShaderStageFlagBits shaderStageType, const char * entryFunc)
{
	auto cleanEnv = []()
	{
		deleteFile("shaderCompilers/glsl/in.shader");
		deleteFile("shaderCompilers/glsl/out.shader");
	};

	{
		std::ofstream inputFile("shaderCompilers/glsl/in.shader");
		inputFile.write(src, len);
		inputFile.close();
	}

	char cmd[1024];
	sprintf_s(cmd, "glslangValidator.exe -V100 -e %s -S %s -o out.shader in.shader", entryFunc, getGLSLangValidatorShaderStage(shaderStageType));

	if (executeCommand(cmd, "shaderCompilers/glsl/"))
	{
		bool result = fromSPIRVFile(device, "shaderCompilers/glsl/out.shader", shaderStageType, entryFunc);
		cleanEnv();
		return result;
	}

	cleanEnv();
	return false;
}

bool ShaderStage::fromSPIRVSource(VkDevice device, const char* src, uint32_t len, VkShaderStageFlagBits shaderStageType, const char* entryFunc)
{
	clear(device);

	if (len <= 4)
		return false;

	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = len;
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(src);

	VkResult result = vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &m_shaderModule);

	m_entryFunction = entryFunc;
	m_shaderType = shaderStageType;
	
	return result == VK_SUCCESS;
}

bool ShaderStage::fromHLSLFile(VkDevice device, const char * hlslShaderFile, VkShaderStageFlagBits shaderStageType, const char * entryFunc)
{
	std::string hlslShaderSrc = convertFileToString(hlslShaderFile);
	return fromGLSLSource(device, hlslShaderSrc.c_str(), static_cast<uint32_t>(hlslShaderSrc.size()), shaderStageType, entryFunc);
}

bool ShaderStage::fromGLSLFile(VkDevice device, const char * glslShaderFile, VkShaderStageFlagBits shaderStageType, const char * entryFunc)
{
	std::string glslShaderSrc = convertFileToString(glslShaderFile);
	return fromGLSLSource(device, glslShaderSrc.c_str(), static_cast<uint32_t>(glslShaderSrc.size()), shaderStageType, entryFunc);
}

bool ShaderStage::fromSPIRVFile(VkDevice device, const char * spirvShaderFile, VkShaderStageFlagBits shaderStageType, const char* entryFunc)
{
	std::string spirvShaderSrc = convertFileToString(spirvShaderFile);
	return fromSPIRVSource(device, spirvShaderSrc.c_str(), static_cast<uint32_t>(spirvShaderSrc.size()), shaderStageType, entryFunc);
}

VkShaderModule ShaderStage::getVkShaderModule() const
{
	return m_shaderModule;
}

VkShaderStageFlagBits ShaderStage::getVkShaderType() const
{
	return m_shaderType;
}

const char* ShaderStage::getEntryFuncName() const
{
	return m_entryFunction.c_str();
}

void ShaderStage::clear(VkDevice device)
{
	if (m_shaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(device, m_shaderModule, nullptr);
	}
	m_shaderModule  = VK_NULL_HANDLE;
	m_entryFunction = "";
}

const char * ShaderStage::getGLSLangValidatorShaderStage(VkShaderStageFlagBits shaderStage)
{
	switch (shaderStage)
	{
		case VK_SHADER_STAGE_VERTEX_BIT                  :  return "vert" ;
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT    :  return "tesc" ;
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT :  return "tese" ;
		case VK_SHADER_STAGE_GEOMETRY_BIT                :  return "geom" ;
		case VK_SHADER_STAGE_FRAGMENT_BIT                :  return "frag" ;
		case VK_SHADER_STAGE_COMPUTE_BIT                 :  return "comp" ;
		case VK_SHADER_STAGE_RAYGEN_BIT_NVX              :	return "rgen" ;
		case VK_SHADER_STAGE_ANY_HIT_BIT_NVX             :	return "rahit";
		case VK_SHADER_STAGE_CLOSEST_HIT_BIT_NVX         :	return "rchit";
		case VK_SHADER_STAGE_MISS_BIT_NVX                :	return "rmiss";
		case VK_SHADER_STAGE_INTERSECTION_BIT_NVX        :	return "rint" ;
		case VK_SHADER_STAGE_CALLABLE_BIT_NVX            :	return "rcall";
		case VK_SHADER_STAGE_TASK_BIT_NV                 :	return "task" ;
		case VK_SHADER_STAGE_MESH_BIT_NV                 :	return "mesh" ;
		default:
			assert(0 && "unknown shader type.");
	}
	return "";
}
