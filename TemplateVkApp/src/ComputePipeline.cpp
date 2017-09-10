#include "ComputePipeline.h"

#include "VkRenderer.h"

ComputePipeline::ComputePipeline(
	const VkRenderer& renderer,
	const std::vector<VkDescriptorSetLayout>& descriptorLayouts,
	VkPipelineShaderStageCreateInfo shaderStageCreateInfo): renderer(renderer)
{
	// ============================
	// Create Pipeline layout
	// ============================
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(int);
	pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	vkCreatePipelineLayout(renderer.getVkDevice(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);

	// ============================
	// Create Pipeline
	// ============================
	VkComputePipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.stage = shaderStageCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = 0;

	vkCreateComputePipelines(renderer.getVkDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
}

VkPipelineLayout ComputePipeline::getPipelineLayout()
{
	return pipelineLayout;
}

VkPipeline ComputePipeline::getPipeline()
{
	return pipeline;
}

ComputePipeline::~ComputePipeline()
{
	if (pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(renderer.getVkDevice(), pipeline, nullptr);
		pipeline = VK_NULL_HANDLE;
	}

	if (pipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(renderer.getVkDevice(), pipelineLayout, nullptr);
		pipelineLayout = VK_NULL_HANDLE;
	}
}
