#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "helper.h"

// Graphics Library - Vulkan
#include "VkRenderer.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include <vulkan/vulkan.h>

#include "GraphicsPipeline.h"
#include "ComputePipeline.h"
#include "Shader.h"

// STD
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <mutex>

//#include "Navigation.h"

struct SAMPLE_POINTS {
    glm::vec4     point[256];
    glm::vec4     random_vectors[256];
};

#define MAX_FRAMEBUFFER_WIDTH 2048
#define MAX_FRAMEBUFFER_HEIGHT 2048

	struct PlyObjVertex
	{
	  glm::vec3 pos;
	  glm::vec3 normal;
	};

	void EventMouseButton(GLFWwindow* window, int button, int action, int mods);
	void EventMousePos(GLFWwindow* window, double xpos, double ypos);
	void EventMouseWheel(GLFWwindow* window, double xoffset, double yoffset);
	void EventKey(GLFWwindow* window, int key, int scancode, int action, int mods);
	void EventChar(GLFWwindow* window, int codepoint);

	void WindowSizeCB(GLFWwindow* window, int width, int height);
	void error_callback(int error, const char* description);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

class Application {
public:
    Application();

    void init(const unsigned int& width, const unsigned int& height);
	void run();
    void shutdown();

    ~Application();

	void EventMouseButton(GLFWwindow* window, int button, int action, int mods);
	void EventMousePos(GLFWwindow* window, double xpos, double ypos);
	void EventMouseWheel(GLFWwindow* window, double xoffset, double yoffset);
	void EventKey(GLFWwindow* window, int key, int scancode, int action, int mods);
	void EventChar(GLFWwindow* window, int codepoint);

	// Callback function called by GLFW when window size changes
	void WindowSizeCB(GLFWwindow* window, int width, int height);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
	void init();
    void create();
    void update(float elapsedTime, float elapsedSinceLastFrame);
    void draw(float elapsedTime, float elapsedSinceLastFrame);

	void computeLoop(float elapsedTime, float elapsedSinceLastFrame);
	void graphicsLoop(float elapsedTime, float elapsedSinceLastFrame);

	void initGraphicsDescriptor();
	void updateGraphicsDescriptorSets();
	void deInitGraphicsDescriptor();
	void initGraphicsPipeline();
	void deInitGraphicsPipeline();

	void initComputeDescriptor();
	void updateComputeDescriptorSets();
	void deInitComputeDescriptor();
	void initComputePipeline();
	void deinitComputePipeline();

private:
	// Key bindings
    bool m_controlKeyHold;
    bool m_altKeyHold;
    bool m_w_pressed, m_s_pressed, m_a_pressed, m_d_pressed, m_q_pressed, m_e_pressed;
    bool m_mouse_left_drag, m_mouse_middle_drag, m_mouse_right_drag;

	// Window related information
    GLFWwindow* m_window;
	double       m_fov;
	double       m_aspectRatio, m_zNear;
	double       m_arcBallRadius;
    unsigned int m_width, m_height;
   
	// Vulkan 
	VkRenderer renderer;

	// Mesh Info
	Buffer vertexBuffer;
	Buffer indexBuffer;
	Buffer transformationBuffer;

	VkShaderModule computeShader = VK_NULL_HANDLE;

	VkDescriptorPool graphicsDescriptorPool	= VK_NULL_HANDLE;
	VkDescriptorSet	graphicsDescriptorSet = VK_NULL_HANDLE;
	VkDescriptorSetLayout graphicsDescriptorSetLayout = VK_NULL_HANDLE;

	VkDescriptorPool computeDescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet	computeDescriptorSet = VK_NULL_HANDLE;
	VkDescriptorSetLayout computeDescriptorSetLayout = VK_NULL_HANDLE;

	VkSemaphore semaphore;
	VkCommandBuffer cmdBuffer;
	VkCommandPool cmdPool;

	std::unique_ptr<GraphicsPipeline> graphicsPipeline;
	std::unique_ptr<ComputePipeline>  computePipeline;

	uint32_t nVertices;
	uint32_t nIndices;
	const uint32_t localWorkGroupSize[3] = { 128, 1, 1 };

	void freeVkMemory();
};

#endif