#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class VkRenderer;

class GraphicsPipeline
{
public:
	GraphicsPipeline(
		const VkRenderer& renderer, 
		const std::vector<VkDescriptorSetLayout>& descriptorLayouts, 
		const std::vector<VkPipelineShaderStageCreateInfo>& shaderStagesCreateInfo,
		const std::vector<VkVertexInputAttributeDescription>& vertexInputAttribDescriptions,
		const std::vector<VkVertexInputBindingDescription>& vertexInputingBindingDecriptions,
		VkPrimitiveTopology primitiveTopology
	);

	VkPipelineLayout getPipelineLayout();
	VkPipeline getPipeline();

	~GraphicsPipeline();

private:
	const VkRenderer& renderer;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
};