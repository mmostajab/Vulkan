#include "Application.h"

#include <array>

void Application::initComputeDescriptor()
{
	std::array<VkDescriptorSetLayoutBinding, 1> bindings{};
	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[0].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = 1;
	descriptorSetLayoutCreateInfo.pBindings = bindings.data();
	vkCreateDescriptorSetLayout(renderer.getVkDevice(), &descriptorSetLayoutCreateInfo, nullptr, &computeDescriptorSetLayout);

	VkDescriptorPoolSize descriptorSetPoolSize{};
	descriptorSetPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorSetPoolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = 1;
	descriptorPoolCreateInfo.pPoolSizes = &descriptorSetPoolSize;
	vkCreateDescriptorPool(renderer.getVkDevice(), &descriptorPoolCreateInfo, nullptr, &computeDescriptorPool);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &computeDescriptorSetLayout;
	descriptorSetAllocateInfo.descriptorPool = computeDescriptorPool;
	vkAllocateDescriptorSets(renderer.getVkDevice(), &descriptorSetAllocateInfo, &computeDescriptorSet);

	updateComputeDescriptorSets();
}

void Application::updateComputeDescriptorSets()
{
	VkDescriptorBufferInfo descriptorBufferInfo{};
	descriptorBufferInfo.buffer = vertexBuffer.vkBuffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = computeDescriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrite.pImageInfo = nullptr;
	descriptorWrite.pBufferInfo = &descriptorBufferInfo;
	descriptorWrite.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(renderer.getVkDevice(), 1, &descriptorWrite, 0, nullptr);
}

void Application::deInitComputeDescriptor()
{
	//vkFreeDescriptorSets(renderer.getVkDevice(), descriptorPool, 1, &descriptorSet);

	vkDestroyDescriptorPool(renderer.getVkDevice(), computeDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(renderer.getVkDevice(), computeDescriptorSetLayout, nullptr);
}

void Application::initComputePipeline()
{
	// ======================================
	// Load the precompiled shaders
	// ======================================
	computeShader = renderer.createShaderModule("../shaders/comp.spv");

	initComputeDescriptor();

	// ============================
	// Pipeline Preparation
	// ============================
	VkPipelineShaderStageCreateInfo shaderStagesCreateInfo{};
	shaderStagesCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStagesCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderStagesCreateInfo.module = computeShader;
	shaderStagesCreateInfo.pName = "main";
	shaderStagesCreateInfo.pSpecializationInfo = nullptr;
	
	// position:
	std::vector<VkVertexInputAttributeDescription> inputAttribDescription(2);
	inputAttribDescription[0].binding = 0;
	inputAttribDescription[0].location = 0;
	inputAttribDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	inputAttribDescription[0].offset = 0;

	// normal:
	inputAttribDescription[1].binding = 0;
	inputAttribDescription[1].location = 1;
	inputAttribDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	inputAttribDescription[1].offset = sizeof(glm::vec3);

	// How the inputs are defined in the buffer.
	VkVertexInputBindingDescription inputBindingDescription{};
	inputBindingDescription.binding = 0;
	inputBindingDescription.stride = sizeof(PlyObjVertex);
	inputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	computePipeline =
		std::unique_ptr<ComputePipeline>(
			new ComputePipeline(renderer, { computeDescriptorSetLayout }, shaderStagesCreateInfo)
			);
}

void Application::deinitComputePipeline()
{
	graphicsPipeline.reset(nullptr);
}