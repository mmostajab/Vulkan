#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class VkRenderer;

class ComputePipeline
{
public:
	ComputePipeline(
		const VkRenderer& renderer,
		const std::vector<VkDescriptorSetLayout>& descriptorLayouts,
		VkPipelineShaderStageCreateInfo shaderStageCreateInfo
	);

	VkPipelineLayout getPipelineLayout();
	VkPipeline getPipeline();

	~ComputePipeline();

private:
	const VkRenderer& renderer;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
};