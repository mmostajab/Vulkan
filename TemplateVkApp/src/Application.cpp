#ifdef WIN32
	#include <Windows.h>
#endif

#include "application.h"
#include "plydatareader.h"
#include "GraphicsPipeline.h"

// STD
#include <iostream>
#include <fstream>
#include <ctime>
#include <random>
#include <array>

// GL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

//#define PORSCHE_MESH
//#define SPHERE_MESH

Application::Application() {
}

void Application::init(const unsigned int& width, const unsigned int& height)
{

    m_width = width; m_height = height;

    glfwSetErrorCallback(::error_callback);
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	if (glfwVulkanSupported() == GLFW_TRUE) 
	{
		std::cout << "Vulkan is supported.\n";
	}
	else 
	{
		std::cout << "Vulkan is not supported.\n";
		exit(-1);
	}

	std::vector<const char*> extensions;
	uint32_t extensionCount;
	const char** extensionsArr = glfwGetRequiredInstanceExtensions(&extensionCount);
	for (uint32_t i = 0; i < extensionCount; i++) extensions.push_back(extensionsArr[i]);
	
	std::vector<const char*> deviceExtensions;
	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	// initializes the renderer.
	renderer.init("VkMeshLoader", extensions, deviceExtensions);
	
	// create a window
    m_window = glfwCreateWindow(width, height, "VkMeshLoader", NULL, NULL);
    if (!m_window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

	// init the surface
	renderer.createWindowSurface(m_window);

    glfwSetKeyCallback(m_window, ::key_callback);
    glfwSetWindowSizeCallback(m_window, ::WindowSizeCB);
    glfwSetCursorPosCallback(m_window, ::EventMousePos);
    glfwSetScrollCallback(m_window, ::EventMouseWheel);
    glfwSetMouseButtonCallback(m_window, (GLFWmousebuttonfun)::EventMouseButton);
    glfwSetKeyCallback(m_window, (GLFWkeyfun)::EventKey);
    // - Directly redirect GLFW char events to AntTweakBar
    glfwSetCharCallback(m_window, (GLFWcharfun)::EventChar);

	// Sets this object as the passed data
	glfwSetWindowUserPointer(m_window, this);

    init();
}

void Application::init() {
	m_fov = glm::pi<double>() / 3.0;
	m_aspectRatio = static_cast<double>(m_width) / m_height;
	m_zNear = 0.01;

	back_color = glm::vec4(0.2f, 0.6f, 0.7f, 1.0f);
	one = 1.0f;

	// Static Members
	m_controlKeyHold = false;
	m_altKeyHold = false;
	m_w_pressed = false;
	m_s_pressed = false;
	m_a_pressed = false;
	m_d_pressed = false;
	m_q_pressed = false;
	m_e_pressed = false;
	m_mouse_left_drag = false;
	m_mouse_middle_drag = false;
	m_mouse_right_drag = false;

	cmdPool = renderer.createCommandPool();
	cmdBuffer = renderer.createCommandBuffer(cmdPool);
	semaphore = renderer.createSemaphore();

	initGraphicsPipeline();
	initComputePipeline();
}

void Application::create() {

#ifdef PORSCHE_MESH
   PlyDataReader::getSingletonPtr()->readDataInfo("../data/big_porsche.ply", nullptr, 0);
#elif defined(SPHERE_MESH)
   PlyDataReader::getSingletonPtr()->readDataInfo("../data/sphere.ply", nullptr, 0);
#else
   PlyDataReader::getSingletonPtr()->readDataInfo("../data/bunny.ply", nullptr, 0);
#endif

   unsigned int nVertices = PlyDataReader::getSingletonPtr()->getNumVertices();
   unsigned int nFaces    = PlyDataReader::getSingletonPtr()->getNumFaces();


   vertices.resize(nVertices+4);
   indices.resize(nFaces * 3+6);
   
   PlyDataReader::getSingletonPtr()->readData(vertices.data(), indices.data());

   glm::vec3 center;
   glm::float32 min_y = vertices[0].pos.y;
   size_t i = 0;
   for (; i < vertices.size()-4; i++) {
     center += vertices[i].pos;
     min_y = glm::min(min_y, vertices[i].pos.y);
   }
   center /= vertices.size();

#ifdef ADD_GROUND
   float width = 400.0f;
   vertices[nVertices + 0].pos = glm::vec3(-width, min_y, -width);
   vertices[nVertices + 0].normal = glm::vec3(0, 1, 0);
   vertices[nVertices + 1].pos = glm::vec3(-width, min_y, width);
   vertices[nVertices + 1].normal = glm::vec3(0, 1, 0);
   vertices[nVertices + 2].pos = glm::vec3( width, min_y, -width);
   vertices[nVertices + 2].normal = glm::vec3(0, 1, 0);
   vertices[nVertices + 3].pos = glm::vec3( width, min_y, width);
   vertices[nVertices + 3].normal = glm::vec3(0, 1, 0);

   indices[3 * nFaces + 0] = nVertices + 0;
   indices[3 * nFaces + 1] = nVertices + 1;
   indices[3 * nFaces + 2] = nVertices + 2;
   indices[3 * nFaces + 3] = nVertices + 2;
   indices[3 * nFaces + 4] = nVertices + 1;
   indices[3 * nFaces + 5] = nVertices + 3;
#endif

   for (size_t i = 0; i < vertices.size(); i++) {
     vertices[i].pos -= center;
   }

   for (size_t i = 0; i < vertices.size(); i++) {

#ifdef PORSCHE_MESH
     vertices[i].pos *= 0.13;
#elif defined(SPHERE_MESH)
     vertices[i].pos *= m_arcBallRadius;
#else
     vertices[i].pos *= 5.0;
#endif
   }

   // ============================================
   // Create Vulkan Buffer for the mesh vertices
   // ============================================
   vertexBuffer = renderer.createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices.size() * sizeof(PlyObjVertex));
   
   // =============================
   // Fill Vertex Buffer
   // =============================
   void* verticesData = nullptr;
   vkMapMemory(renderer.getVkDevice(), vertexBuffer.vkBufferMemory, 0, VK_WHOLE_SIZE, 0, &verticesData);
   for (size_t i = 0; i < vertices.size(); i++) {
	   PlyObjVertex& vertex = ((PlyObjVertex*)verticesData)[i];
	   vertex.pos		= vertices[i].pos;
	   vertex.normal	= vertices[i].normal;
   }
   vkUnmapMemory(renderer.getVkDevice(), vertexBuffer.vkBufferMemory);

   // ============================================
   // Create Vulkan Buffer for the mesh indices
   // ============================================
   indexBuffer = renderer.createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices.size() * sizeof(unsigned int));

   // =============================
   // Fill Index Buffer
   // =============================
   void* indicesData = nullptr;
   vkMapMemory(renderer.getVkDevice(), indexBuffer.vkBufferMemory, 0, VK_WHOLE_SIZE, 0, &indicesData);
   for (size_t i = 0; i < indices.size(); i++) {
	   unsigned int& index = ((unsigned int*)indicesData)[i];
	   index = indices[i];
   }
   vkUnmapMemory(renderer.getVkDevice(), indexBuffer.vkBufferMemory);
}

void Application::update(float time, float timeSinceLastFrame) {

	// ===========================================
	// Set the transformations uniform buffer
	// ===========================================
	glm::mat4 identity(1.0f);

	glm::mat4 viewMatrix = glm::lookAt(glm::vec3(sin(time), 0.0f, cos(time)), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	//glm::mat4 projMatrix = glm::ortho(-1.0f, 1.0f, 1.0f, -1.0f, -20.0f, 20.0f);
	glm::mat4 projMatrix = glm::perspectiveFov(glm::pi<float>() / 2.0f, (float)renderer.getVkSurfaceWidth(), (float)renderer.getVkSurfaceHeight(), 0.001f, 1000.0f);

	void* trasnformations_ = nullptr;
	vkMapMemory(renderer.getVkDevice(), transformationBuffer.vkBufferMemory, 0, VK_WHOLE_SIZE, 0, &trasnformations_);
	
	glm::mat4* transformations = (glm::mat4*)trasnformations_;
	transformations[0] = projMatrix;
	transformations[1] = viewMatrix;
	transformations[2] = identity;

	VkMappedMemoryRange memoryRange{};
	memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	memoryRange.memory = transformationBuffer.vkBufferMemory;
	memoryRange.offset = 0;
	memoryRange.size = VK_WHOLE_SIZE;
	
	vkFlushMappedMemoryRanges(renderer.getVkDevice(), 1, &memoryRange);

	vkUnmapMemory(renderer.getVkDevice(), transformationBuffer.vkBufferMemory);
}

void Application::draw() {
}

void Application::drawPly() {
}

void Application::freeVkMemory()
{
	renderer.destroyBuffer(vertexBuffer);
	renderer.destroyBuffer(indexBuffer);
	renderer.destroyBuffer(transformationBuffer);
}

void Application::run() {
	create();
	double start_time;
	double start_frame, end_frame;
	start_time = start_frame = glfwGetTime();
	uint64_t frame_counter = 0;

	while (!glfwWindowShouldClose(m_window))
	{
		// ========================
		// CPU update buffers.
		// ========================

		// Process OS events.
		glfwPollEvents();

		frame_counter++;
		double now_time = glfwGetTime();
		if (frame_counter % 100 == 0) std::cout << "FPS = " << 1.0 / (end_frame - start_frame) << std::endl;
		update(static_cast<float>(now_time - start_time), static_cast<float>(now_time - start_frame));

		start_frame = glfwGetTime();

		// ========================
		// Begin Render
		// ========================
		renderer.beginRender();

		// ========================
		// Record Command Buffer
		// ========================
		VkCommandBufferBeginInfo cmdBufferBeginInfo{};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo);

		VkMemoryBarrier transformationMemBarrier{};
		transformationMemBarrier.sType			= VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		transformationMemBarrier.srcAccessMask	= VK_ACCESS_HOST_WRITE_BIT;
		transformationMemBarrier.dstAccessMask	= VK_ACCESS_UNIFORM_READ_BIT;

		vkCmdPipelineBarrier(
			cmdBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
			0, 1, &transformationMemBarrier, 0, nullptr, 0, nullptr);

		VkRect2D renderArea{};
		renderArea.offset.x			= 0;
		renderArea.offset.y			= 0;
		renderArea.extent.width		= renderer.getVkSurfaceWidth();
		renderArea.extent.height	= renderer.getVkSurfaceHeight();

		// First Attachment:	Depth-Stencil
		// Second Attachment:	Color
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].depthStencil.depth	= 1.0f; 
		clearValues[0].depthStencil.stencil = 0;
		clearValues[1].color.float32[0]		= static_cast<float>(2.0 * sin((glfwGetTime() - start_time) * 0.5) - 1.0);
		clearValues[1].color.float32[1]		= static_cast<float>(2.0 * sin((glfwGetTime() - start_time) * 1.0) - 1.0);
		clearValues[1].color.float32[2]		= static_cast<float>(2.0 * sin((glfwGetTime() - start_time) * 1.5) - 1.0);
		clearValues[1].color.float32[3]		= 1.0f;

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass		= renderer.getVkRenderPass();
		renderPassBeginInfo.framebuffer		= renderer.getVkActiveFrameBuffer();
		renderPassBeginInfo.renderArea		= renderArea;
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBeginInfo.pClearValues	= clearValues.data();
				
		vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);



		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getPipeline());

		VkViewport viewport{};
		viewport.x			= 0;
		viewport.y			= 0;
		viewport.width		= static_cast<float>(renderer.getVkSurfaceWidth());
		viewport.height		= static_cast<float>(renderer.getVkSurfaceHeight());
		viewport.minDepth	= -1.0f;
		viewport.maxDepth	= 1.0f;
		
		VkRect2D scissor{};
		scissor.extent.width	= renderer.getVkSurfaceWidth();
		scissor.extent.height	= renderer.getVkSurfaceHeight();
		scissor.offset.x		= 0;
		scissor.offset.y		= 0;

		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

		VkDeviceSize noOffset = 0;
		
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &vertexBuffer.vkBuffer, &noOffset);
		vkCmdBindIndexBuffer(cmdBuffer, indexBuffer.vkBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(cmdBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(cmdBuffer);

		vkEndCommandBuffer(cmdBuffer);

		// ========================
		// Submit Command Buffer
		// ========================
		VkSubmitInfo submitInfo{};
		submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount	= 0;
		submitInfo.pWaitSemaphores		= nullptr;
		submitInfo.pWaitDstStageMask	= nullptr;
		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &cmdBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores	= &semaphore;

		vkQueueSubmit(renderer.getVkQueue(), 1, &submitInfo, VK_NULL_HANDLE);

		// ========================
		// End Render		
		// ========================
		renderer.endRender({ semaphore });

		end_frame = glfwGetTime();
	}	
}

void Application::shutdown() {

	// Wait until the commands in the queue are done before starting the deinitialization.
	vkQueueWaitIdle(renderer.getVkQueue());

	vkDestroySemaphore(renderer.getVkDevice(), semaphore, nullptr);
	vkDestroyCommandPool(renderer.getVkDevice(), cmdPool, nullptr);

	deinitComputePipeline();
	deInitGraphicsPipeline();
	freeVkMemory();
	deInitDescriptor();

	// ==================================
	// Free up the shaders
	// ==================================
	vkDestroyShaderModule(renderer.getVkDevice(), vertexShader,   nullptr);
	vkDestroyShaderModule(renderer.getVkDevice(), fragmentShader, nullptr);

	renderer.deInit();
	glfwDestroyWindow(m_window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

Application::~Application() {
}

void Application::initDescriptor()
{
	std::array<VkDescriptorSetLayoutBinding, 1> bindings{};
	bindings[0].binding				= 0;
	bindings[0].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].descriptorCount		= 1;
	bindings[0].stageFlags			= VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[0].pImmutableSamplers	= nullptr;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount	= 1;
	descriptorSetLayoutCreateInfo.pBindings		= bindings.data();
	vkCreateDescriptorSetLayout(renderer.getVkDevice(), &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);

	VkDescriptorPoolSize descriptorSetPoolSize{};
	descriptorSetPoolSize.type				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetPoolSize.descriptorCount	= 1;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.flags				= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.maxSets			= 1;
	descriptorPoolCreateInfo.poolSizeCount		= 1;
	descriptorPoolCreateInfo.pPoolSizes			= &descriptorSetPoolSize;
	vkCreateDescriptorPool(renderer.getVkDevice(), &descriptorPoolCreateInfo, nullptr, &descriptorPool);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	vkAllocateDescriptorSets(renderer.getVkDevice(), &descriptorSetAllocateInfo, &descriptorSet);

	VkDescriptorBufferInfo descriptorBufferInfo{};
	descriptorBufferInfo.buffer = transformationBuffer.vkBuffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range	= VK_WHOLE_SIZE;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet				= descriptorSet;
	descriptorWrite.dstBinding			= 0;
	descriptorWrite.dstArrayElement		= 0;
	descriptorWrite.descriptorCount		= 1;
	descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.pImageInfo			= nullptr;
	descriptorWrite.pBufferInfo			= &descriptorBufferInfo;
	descriptorWrite.pTexelBufferView	= nullptr;

	vkUpdateDescriptorSets(renderer.getVkDevice(), 1, &descriptorWrite, 0, nullptr);
}

void Application::deInitDescriptor()
{
	//vkFreeDescriptorSets(renderer.getVkDevice(), descriptorPool, 1, &descriptorSet);

	vkDestroyDescriptorPool(renderer.getVkDevice(), descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(renderer.getVkDevice(), descriptorSetLayout, nullptr);
}

void Application::initComputePipeline()
{
	// ============================
	// Create Pipeline layout
	// ============================
	/*VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	vkCreatePipelineLayout(renderer.getVkDevice(), &pipelineLayoutCreateInfo, nullptr, &computePipelineLayout);

	std::unique_ptr<VkPipeline> computePipeline = std::make_unique<VkPipeline>();

	VkComputePipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = VK_NULL_HANDLE;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = VK_NULL_HANDLE;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = VK_NULL_HANDLE;
	
	vkCreateComputePipelines(renderer.getVkDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, computePipeline.get());*/

}

void Application::deinitComputePipeline()
{
	graphicsPipeline.reset(nullptr);
}

#include <memory>

void Application::initGraphicsPipeline()
{
	// ======================================
	// Load the precompiled shaders
	// ======================================
	vertexShader = renderer.createShaderModule("../shaders/vert.spv");
	fragmentShader = renderer.createShaderModule("../shaders/frag.spv");

	transformationBuffer = renderer.createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 3 * sizeof(glm::mat4));

	initDescriptor();

	// ============================
	// Pipeline Preparation
	// ============================
	std::vector<VkPipelineShaderStageCreateInfo> shaderStagesCreateInfo(2);
	shaderStagesCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStagesCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStagesCreateInfo[0].module = vertexShader;
	shaderStagesCreateInfo[0].pName = "main";
	shaderStagesCreateInfo[0].pSpecializationInfo = nullptr;

	shaderStagesCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStagesCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStagesCreateInfo[1].module = fragmentShader;
	shaderStagesCreateInfo[1].pName = "main";
	shaderStagesCreateInfo[1].pSpecializationInfo = nullptr;

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

	graphicsPipeline = 
		std::unique_ptr<GraphicsPipeline>(
			new GraphicsPipeline( renderer, { descriptorSetLayout }, shaderStagesCreateInfo, 
				inputAttribDescription, { inputBindingDescription }, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ) 
		);
}

void Application::deInitGraphicsPipeline()
{
	graphicsPipeline.reset(nullptr);
}

void Application::EventMouseButton(GLFWwindow* window, int button, int action, int mods) {

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		// Get the current mouse cursor position
		glm::dvec2 currentMousePos;
		glfwGetCursorPos(m_window, &currentMousePos[0], &currentMousePos[1]);

		m_mouse_left_drag = true;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		m_mouse_left_drag = false;
	}

	if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		glm::dvec2 currentMousePos;
		glfwGetCursorPos(m_window, &currentMousePos[0], &currentMousePos[1]);
		m_mouse_right_drag = true;
	}

	if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		//m_navigation.endPan();
		m_mouse_right_drag = false;
	}

  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    m_mouse_middle_drag = true;

  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    m_mouse_middle_drag = false;
}

void Application::EventMousePos(GLFWwindow* window, double xpos, double ypos) {
	glm::dvec2 newMousePos = { xpos, ypos };

	if (m_mouse_left_drag) {
	}

	if (m_mouse_right_drag) {  
	}
}

void Application::EventMouseWheel(GLFWwindow* window, double xoffset, double yoffset) {
	if(yoffset)
	{
		glm::dvec2 currentMousePos;
		glfwGetCursorPos(m_window, &currentMousePos[0], &currentMousePos[1]);
		glm::ivec2 screenSize;
		glfwGetWindowSize(m_window, &screenSize[0], &screenSize[1]);
		currentMousePos[1] = screenSize[1] - currentMousePos[1];
		currentMousePos += .5;
		currentMousePos /= screenSize;
		currentMousePos = currentMousePos * 2. - 1.;
	}
}

void Application::EventKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS){
    if (key == GLFW_KEY_W)  m_w_pressed = true;
    if (key == GLFW_KEY_S)  m_s_pressed = true;
    if (key == GLFW_KEY_A)  m_a_pressed = true;
    if (key == GLFW_KEY_D)  m_d_pressed = true;
    if (key == GLFW_KEY_Q)  m_q_pressed = true;
    if (key == GLFW_KEY_E)  m_e_pressed = true;

    if (key == GLFW_KEY_LEFT_CONTROL)           m_controlKeyHold = true;
    if (key == GLFW_KEY_LEFT_ALT)               m_altKeyHold     = true;
  }

  if (action == GLFW_RELEASE){
    if (key == GLFW_KEY_W)  m_w_pressed = false;
    if (key == GLFW_KEY_S)  m_s_pressed = false;
    if (key == GLFW_KEY_A)  m_a_pressed = false;
    if (key == GLFW_KEY_D)  m_d_pressed = false;
    if (key == GLFW_KEY_Q)  m_q_pressed = false;
    if (key == GLFW_KEY_E)  m_e_pressed = false;

    if (key == GLFW_KEY_LEFT_CONTROL)           m_controlKeyHold    = false;
    if (key == GLFW_KEY_LEFT_ALT)               m_altKeyHold        = false;

    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);
  }
}

void Application::EventChar(GLFWwindow * window, int codepoint)
{
}

// Callback function called by GLFW when window size changes
void Application::WindowSizeCB(GLFWwindow* window, int width, int height) {

	// Wait until the commands in the queue are done before starting the deinitialization.
	vkQueueWaitIdle(renderer.getVkQueue());
	vkDeviceWaitIdle(renderer.getVkDevice());

	m_width = width; m_height = height;
	m_aspectRatio = static_cast<double>(width) / static_cast<double>(height);

	renderer.destroySurface();
	renderer.createWindowSurface(window);
}

void Application::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, VK_TRUE);

  if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS){
    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);
    //m_camera.Move2D(static_cast<int>(xpos), static_cast<int>(ypos));
    m_controlKeyHold = true;
  }

  if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE){
    m_controlKeyHold = false;
  }
}


// ============================================================================
// Static Global definition of GLFW handlers
// ============================================================================
void EventMouseButton(GLFWwindow* window, int button, int action, int mods) {
	if (auto self = static_cast<Application*>(glfwGetWindowUserPointer(window))) {
		self->EventMouseButton(window, button, action, mods);
	}
}

void EventMousePos(GLFWwindow* window, double xpos, double ypos) {
	if (auto self = static_cast<Application*>(glfwGetWindowUserPointer(window))) {
		self->EventMousePos(window, xpos, ypos);
	}
}

void EventMouseWheel(GLFWwindow* window, double xoffset, double yoffset) {
	if (auto self = static_cast<Application*>(glfwGetWindowUserPointer(window))) {
		self->EventMouseWheel(window, xoffset, yoffset);
	}
}

void EventKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (auto self = static_cast<Application*>(glfwGetWindowUserPointer(window))) {
		self->EventKey(window, key, scancode, action, mods);
	}
}

void EventChar(GLFWwindow* window, int codepoint) {
	if (auto self = static_cast<Application*>(glfwGetWindowUserPointer(window))) {
		self->EventChar(window, codepoint);
	}
}

void WindowSizeCB(GLFWwindow* window, int width, int height) {
	if (auto self = static_cast<Application*>(glfwGetWindowUserPointer(window))) {
		self->WindowSizeCB(window, width, height);
	}
}

void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (auto self = static_cast<Application*>(glfwGetWindowUserPointer(window))) {
		self->key_callback(window, key, scancode, action, mods);
	}
}