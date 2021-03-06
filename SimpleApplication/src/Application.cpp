#ifdef WIN32
	#include <Windows.h>
#endif

#include "application.h"
#include "plydatareader.h"

// STD
#include <iostream>
#include <fstream>
#include <ctime>
#include <random>
#include <array>

// GL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define PORSCHE_MESH
//#define SPHERE_MESH

Application::Application() {
}

void Application::init(const unsigned int& width, const unsigned int& height) {

    m_width = width; m_height = height;

    glfwSetErrorCallback(::error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);

	if (glfwVulkanSupported() == GLFW_TRUE) {
		std::cout << "Vulkan is supported.\n";
	}
	else {
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
	renderer.init("SimpleVulkanApplication", extensions, deviceExtensions);
	
	// create a window
    m_window = glfwCreateWindow(width, height, "Simple Vulkan Application", NULL, NULL);
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
    m_arcBallRadius = 0.7;

	m_navigation.setProject(m_fov, m_aspectRatio, m_zNear);
	m_navigation.setView(glm::dvec3(1.0), glm::dvec3(0.0), glm::dvec3(0.0, 1.0, 0.0));
	m_navigation.setScreenSize(m_width, m_height);

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
}

void Application::create() {
   compileShaders();

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
}

void Application::update(float time, float timeSinceLastFrame) {
}

void Application::draw() {
}

void Application::drawPly() {
}

void Application::setMousePosRepresentation(glm::vec3 mousePointerViewPos)
{
	glm::vec4 mousePointerWorldPos = m_navigation.getInvView() * glm::vec4(mousePointerViewPos, 1.0f);

#if 0
	std::cout << "mouse Pointer Pos = " << mousePointerViewPos[0] << " " << mousePointerViewPos[1] << " " << mousePointerViewPos[2] << std::endl;
#endif
}

void Application::drawMousePointer()
{	
}

float Application::readDepthBuffer(glm::dvec2 mousePos)
{
	float fDepth = 0.0f;
	return fDepth;
}

void Application::run() {
	create();
	double start_time;
	double start_frame, end_frame;
	start_time = start_frame = glfwGetTime();
	uint64_t frame_counter = 0;

	// ==========================================
	// Command Pool Creation
	// ==========================================
	VkCommandPool	cmdPool					= VK_NULL_HANDLE;
	VkCommandBuffer cmdBuffer				= VK_NULL_HANDLE;

	VkCommandPoolCreateInfo cmdPoolCreateInfo{};
	cmdPoolCreateInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.flags					= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolCreateInfo.queueFamilyIndex		= renderer.getVkGraphicsQueueFamilyIndex();
	vkCreateCommandPool(renderer.getVkDevice(), &cmdPoolCreateInfo, nullptr, &cmdPool);

	VkCommandBufferAllocateInfo cmdBufferAllocateInfo{};
	cmdBufferAllocateInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocateInfo.commandPool			= cmdPool;
	cmdBufferAllocateInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocateInfo.commandBufferCount	= 1;

	vkAllocateCommandBuffers(renderer.getVkDevice(), &cmdBufferAllocateInfo, &cmdBuffer);

	VkSemaphore semaphore = VK_NULL_HANDLE;
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(renderer.getVkDevice(), &semaphoreCreateInfo, nullptr, &semaphore);

	while (!glfwWindowShouldClose(m_window))
	{
		// ========================
		// CPU update buffers.
		// ========================
		frame_counter++;
		if (frame_counter % 100 == 0) std::cout << "FPS = " << 1.0 / (end_frame - start_frame) << std::endl;

		// Process OS events.
		glfwPollEvents();

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

	// Wait until the commands in the queue are done before starting the deinitialization.
	vkQueueWaitIdle(renderer.getVkQueue());

	vkDestroySemaphore(renderer.getVkDevice(), semaphore, nullptr);
	vkDestroyCommandPool(renderer.getVkDevice(), cmdPool, nullptr);
}

void Application::shutdown() {
	renderer.deInit();
	glfwDestroyWindow(m_window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

Application::~Application() {
}

void Application::compileShaders() { 
}

void Application::EventMouseButton(GLFWwindow* window, int button, int action, int mods) {

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		// Get the current mouse cursor position
		glm::dvec2 currentMousePos;
		glfwGetCursorPos(m_window, &currentMousePos[0], &currentMousePos[1]);

		glm::dmat4 viewMat = m_navigation.getView();
		glm::dvec3 viewCenterOfRotation(0.0);
		viewCenterOfRotation = glm::dvec3(viewMat * glm::dvec4(viewCenterOfRotation, 1.0));

		float depth = readDepthBuffer(currentMousePos);

		setMousePosRepresentation(m_navigation.getPointViewCoord(currentMousePos, readDepthBuffer(currentMousePos)));

		glm::dvec3 viewcurrentMousePos = m_navigation.getPointViewCoord(currentMousePos, depth);
		double viewSpaceRadius = glm::length(viewcurrentMousePos - viewCenterOfRotation);

//		setMousePosRepresentation(viewSpaceRadius);


#if 0
		std::cout << "=================================================================\n";
		std::cout << "   Initial State: \n";
		std::cout << "viewCenterOfRotation = " << viewCenterOfRotation[0] << " " << viewCenterOfRotation[1] << " " << viewCenterOfRotation[2] << std::endl;
		std::cout << "viewcurrentMousePos = " << viewcurrentMousePos[0] << " " << viewcurrentMousePos[1] << " " << viewcurrentMousePos[2] << std::endl;
		std::cout << "viewSpaceRadius = " << viewSpaceRadius << std::endl;
		std::cout << "=================================================================\n";
#endif

		m_navigation.startUpdate(currentMousePos, viewCenterOfRotation/*viewcurrentMousePos*/, viewSpaceRadius);
		m_mouse_left_drag = true;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		m_navigation.endUpdate();
		m_mouse_left_drag = false;
	}

	if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		glm::dvec2 currentMousePos;
		glfwGetCursorPos(m_window, &currentMousePos[0], &currentMousePos[1]);

		float depth = readDepthBuffer(currentMousePos);

		setMousePosRepresentation(m_navigation.getPointViewCoord(currentMousePos, readDepthBuffer(currentMousePos)));
		glm::dvec3 viewcurrentMousePos = m_navigation.getPointViewCoord(currentMousePos, depth);

		m_navigation.startPan(viewcurrentMousePos);
		m_mouse_right_drag = true;
	}

	if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		m_navigation.endPan();
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
	  m_navigation.updateRotate(newMousePos);

	  ;

	  m_prevMousePos = newMousePos;
  }

  if (m_mouse_right_drag) {  
	  m_navigation.updatePan(newMousePos);
  }

  /*if (m_mouse_right_drag) {
    m_camera.OffsetFrustum(static_cast<int>(xpos), static_cast<int>(ypos));
  }

  m_camera.SetMousePos(static_cast<int>(xpos), static_cast<int>(ypos));*/
}

void Application::EventMouseWheel(GLFWwindow* window, double xoffset, double yoffset) {
  //m_camera.MoveForward(static_cast<int>(yoffset));
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
		m_navigation.zoomStep(yoffset < 0 ? -1 : 1, currentMousePos);
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
    //m_camera.SetMousePos(static_cast<int>(xpos), static_cast<int>(ypos));
  }
}

void Application::EventChar(GLFWwindow * window, int codepoint)
{
}

// Callback function called by GLFW when window size changes
void Application::WindowSizeCB(GLFWwindow* window, int width, int height) {
  m_width = width; m_height = height;
  //glViewport(0, 0, width, height);
  m_aspectRatio = static_cast<double>(width) / static_cast<double>(height);
  m_navigation.setProject(m_fov, m_aspectRatio, m_zNear);
  m_navigation.setScreenSize(m_width, m_height);
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