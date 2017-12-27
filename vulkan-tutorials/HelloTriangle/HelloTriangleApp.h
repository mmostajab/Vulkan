#pragma once

#include <vulkan/vulkan.hpp>

#include "VkDeleter.h"
#include "Mesh.h"
#include "CommonStructs.h"

#include <array>
#include <memory>

struct SwapChainSupportDeatils
{
	VkSurfaceCapabilitiesKHR caps;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

std::string GetAPIVersionString(uint32_t apiVersion);
void DestroyDbgReportCallBack(VkInstance instance, VkDebugReportCallbackEXT callback, VkAllocationCallbacks* pAllocator);

struct QueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete()
	{
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

class HelloTriangleApp
{
public:
	void run();

	bool debugCallBack(
		VkDebugReportFlagBitsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location,
		int32_t code, const char* layerPerfix, const char* msg
	);

private:
	void initWindow();
	void initVk(const std::vector<const char*>& ReqLayers, const std::vector<const char*>& ReqExt);
	void loop();
	void drawFrame();
	void setupDebugCallback(VkAllocationCallbacks* pAllocator);
	void pickPhysicalDeviceAndCreateLogicalDevice(const std::vector<const char*>& ReqLayers, const std::vector<const char*>& ReqExt);
	void createSurface();
	void createSwapChain();
	void createSwapChainImageViews();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createDepthResources();
	void createTextureImages();
	void createTextureImageViews();
	void createTextureSamplers();
	void createVertexBuffers();
	void createIndexBuffers();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	void createCommandBuffers();
	void createSemaphores();

	void recreateSwapChain();
	void updateUniformBuffers();

	bool checkInstanceExtensionsSupport(const std::vector<const char*>& requestedExtensions);
	bool checkInstanceValidationLayersSupport(const std::vector<const char*>& requestedValidationLayers);
	bool checkDeviceExtensionsSupport(VkPhysicalDevice physicalDevice, const std::vector<const char*>& requestedExtensions);
	bool checkDeviceValidationLayersSupport(VkPhysicalDevice physicalDevice, const std::vector<const char*>& requestedValidationLayers);
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
	SwapChainSupportDeatils querySwapChainSupportDetails(VkPhysicalDevice physicalDevice);
	VkSurfaceFormatKHR chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapChainPresentationMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& caps);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice);
	VkFormat findDepthFormat();
	bool hasStencilComponent(VkFormat format);
	
	GLFWwindow* window;
	VkDeleter<VkInstance> instance{ vkDestroyInstance };
	VkPhysicalDevice physicalDevice;
	VkDeleter<VkDevice> device{ vkDestroyDevice };
	VkDeleter<VkDebugReportCallbackEXT> dbgReportcallback{ instance, DestroyDbgReportCallBack };
	VkDeleter<VkSurfaceKHR> surface{ instance, vkDestroySurfaceKHR };
	VkDeleter<VkSwapchainKHR> swapChain{ device, vkDestroySwapchainKHR };
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkDeleter<VkImageView>> swapChainImageViews;
	VkDeleter<VkRenderPass> renderPass{ device, vkDestroyRenderPass };
	VkDeleter<VkPipelineLayout> pipelineLayout{ device, vkDestroyPipelineLayout };
	VkDeleter<VkPipeline> pipeline{ device, vkDestroyPipeline };
	std::vector<VkDeleter<VkFramebuffer>> swapChainFrameBuffers;
	VkDeleter<VkSemaphore> imageAvailableSemaphore{ device, vkDestroySemaphore };
	VkDeleter<VkSemaphore> renderDoneSemaphore{ device, vkDestroySemaphore };

	VkDeleter<VkCommandPool> cmdPool{ device, vkDestroyCommandPool };
	std::vector<VkCommandBuffer> cmdBuffers;

	VkDeleter<VkDescriptorPool> descPool{ device, vkDestroyDescriptorPool };
	VkDeleter<VkDescriptorSetLayout> descSetLayout{ device, vkDestroyDescriptorSetLayout };
	VkDescriptorSet descSet;
	
	VkQueue graphicsQueue;

	uint32_t width = 800;
	uint32_t height = 600;

	ModelViewProjUBO modelViewProjUBO;
	VkDeleter<VkBuffer> modelViewProjStaggingUBOId{ device, vkDestroyBuffer };
	VkDeleter<VkDeviceMemory> modelViewProjStaggingUBOMem{ device, vkFreeMemory };
	VkDeleter<VkBuffer> modelViewProjUBOId{ device, vkDestroyBuffer };
	VkDeleter<VkDeviceMemory> modelViewProjUBOMem{device, vkFreeMemory};
	VkDeleter<VkSampler> linearTextureSampler{ device, vkDestroySampler };

	VkFormat depthFormat;
	VkDeleter<VkImage> depthImg{ device, vkDestroyImage };
	VkDeleter<VkDeviceMemory> depthImgMemory;
	VkDeleter<VkImageView> depthImgView{ device, vkDestroyImageView };

	std::unique_ptr<Mesh> mesh;
	std::string modelPath = "models/chalet.obj";
	std::string texturePath = "textures/chalet.jpg";
	void loadMesh();

#ifdef _DEBUG
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif
};

