#include "Application.h"

#include <array>

void Application::initGraphicsDescriptor()
{
	std::array<VkDescriptorSetLayoutBinding, 1> bindings{};
	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[0].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = 1;
	descriptorSetLayoutCreateInfo.pBindings = bindings.data();
	vkCreateDescriptorSetLayout(renderer.getVkDevice(), &descriptorSetLayoutCreateInfo, nullptr, &graphicsDescriptorSetLayout);

	VkDescriptorPoolSize descriptorSetPoolSize{};
	descriptorSetPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetPoolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = 1;
	descriptorPoolCreateInfo.pPoolSizes = &descriptorSetPoolSize;
	vkCreateDescriptorPool(renderer.getVkDevice(), &descriptorPoolCreateInfo, nullptr, &graphicsDescriptorPool);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &graphicsDescriptorSetLayout;
	descriptorSetAllocateInfo.descriptorPool = graphicsDescriptorPool;
	vkAllocateDescriptorSets(renderer.getVkDevice(), &descriptorSetAllocateInfo, &graphicsDescriptorSet);

	updateGraphicsDescriptorSets();
}

void Application::updateGraphicsDescriptorSets()
{
	VkDescriptorBufferInfo descriptorBufferInfo{};
	descriptorBufferInfo.buffer = transformationBuffer.getVkBuffer();
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = graphicsDescriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.pImageInfo = nullptr;
	descriptorWrite.pBufferInfo = &descriptorBufferInfo;
	descriptorWrite.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(renderer.getVkDevice(), 1, &descriptorWrite, 0, nullptr);
}

void Application::deInitGraphicsDescriptor()
{
	//vkFreeDescriptorSets(renderer.getVkDevice(), descriptorPool, 1, &descriptorSet);

	vkDestroyDescriptorPool(renderer.getVkDevice(), graphicsDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(renderer.getVkDevice(), graphicsDescriptorSetLayout, nullptr);
}

void Application::initGraphicsPipeline()
{
	// ======================================
	// Load the precompiled shaders
	// ======================================

	transformationBuffer = renderer.createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 3 * sizeof(glm::mat4));

	initGraphicsDescriptor();

	// ============================
	// Pipeline Preparation
	// ============================
	ShaderStage vertexShader;
	ShaderStage fragmentShader;

	vertexShader.fromGLSLFile(renderer.getVkDevice(), "glsl/ply.vert", VK_SHADER_STAGE_VERTEX_BIT, "main");
	assert(vertexShader.getVkShaderType() == VK_SHADER_STAGE_VERTEX_BIT);
	fragmentShader.fromGLSLFile(renderer.getVkDevice(), "glsl/ply.frag", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
	assert(fragmentShader.getVkShaderType() == VK_SHADER_STAGE_FRAGMENT_BIT);

	// position:
	std::vector<VkVertexInputAttributeDescription> inputAttribDescription(2);
	inputAttribDescription[0].binding  = 0;
	inputAttribDescription[0].location = 0;
	inputAttribDescription[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
	inputAttribDescription[0].offset   = 0;

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

	graphicsPipeline =
		std::unique_ptr<GraphicsPipeline>(
			new GraphicsPipeline(renderer, { graphicsDescriptorSetLayout }, { vertexShader, fragmentShader },
				inputAttribDescription, { inputBindingDescription }, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			);
}

void Application::deInitGraphicsPipeline()
{
	graphicsPipeline.reset(nullptr);
}