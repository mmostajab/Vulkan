#include "stdafx.h"
#include "HelloTriangleApp.h"
#include "ShaderHelper.h"
#include "VkHelper.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <iomanip>

void HelloTriangleApp::run()
{
	initWindow();

	std::vector<const char*> requestedValidationLayers =
	{
		"VK_LAYER_LUNARG_standard_validation",
	};

	std::vector<const char*> requestedExtensions =
	{
		"VK_EXT_debug_report",
	};

	uint32_t glfwReqExtensionsCount = 0;
	auto glfwReqExtensions = glfwGetRequiredInstanceExtensions(&glfwReqExtensionsCount);
	for (uint32_t extIdx = 0; extIdx < glfwReqExtensionsCount; ++extIdx)
	{
		requestedExtensions.push_back(glfwReqExtensions[extIdx]);
	}

	loadMesh();

	initVk(requestedValidationLayers, requestedExtensions);
	setupDebugCallback(nullptr);
	createSurface();
	pickPhysicalDeviceAndCreateLogicalDevice({}, { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
	createSwapChain();
	createSwapChainImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createCommandPool();
	createDepthResources();
	createFramebuffers();
	createTextureImages();
	createTextureImageViews();
	createTextureSamplers();
	createVertexBuffers();
	createIndexBuffers();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
	createSemaphores();
	
	loop();
}

void HelloTriangleApp::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	auto ptr = this;
	window = glfwCreateWindow(width, height, "Vulkan-Tutorials : Hello Triangle", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, 
		[](GLFWwindow* window, int width, int height) -> void {
			if (width == 0 || height == 0) return;
			auto app = reinterpret_cast<HelloTriangleApp*>(glfwGetWindowUserPointer(window));
			app->width = width;
			app->height = height;
			app->recreateSwapChain();
		}
	);

}

void HelloTriangleApp::initVk(const std::vector<const char*>& ReqLayers, const std::vector<const char*>& ReqExt)
{
	if (!checkInstanceValidationLayersSupport(ReqLayers) || !checkInstanceExtensionsSupport(ReqExt))
	{
		std::runtime_error("Requested validation layer or extension is not supported! See warnings for more details.\n");
		return;
	}

	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 21);
	applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	applicationInfo.pApplicationName = "HelloTriangle";

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(ReqExt.size());
	instanceCreateInfo.ppEnabledExtensionNames = ReqExt.data();
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ReqLayers.size());
	instanceCreateInfo.ppEnabledLayerNames = ReqLayers.data();

	// Magically, make you able to debug instance creation.
	//instanceCreateInfo.pNext = &debugReportCallbackCreateInfo;

	if (vkCreateInstance(&instanceCreateInfo, nullptr, instance.replace()) != VK_SUCCESS)
	{
		std::cerr << "Cannot create the vulkan instance.\n" ;
	}
}

void HelloTriangleApp::pickPhysicalDeviceAndCreateLogicalDevice(const std::vector<const char*>& ReqLayers, const std::vector<const char*>& ReqExt)
{
	uint32_t physicalDeviceCount = 0;
	std::vector<VkPhysicalDevice> physicalDevices;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
	physicalDevices.resize(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

	std::cout << "Available physical devices: \n";
	for (uint32_t i = 0; i < physicalDeviceCount; ++i)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties);
		std::cout
			<< "(" << i << ")\t"
			<< std::setw(10) << physicalDeviceProperties.deviceName
			<< std::setw(10) << "(" << GetAPIVersionString(physicalDeviceProperties.apiVersion) << ")"
			<< std::setw(10) << "(" << GetAPIVersionString(physicalDeviceProperties.driverVersion) << ")"
			<< std::setw(8) << (physicalDeviceProperties.vendorID == 4098 ? "AMD" : "Not AMD")
			<< std::setw(12) << (physicalDeviceProperties.deviceType == 2 ? "Discrete" : "Not Discrete")
			<< std::endl;
	}

	bool isSuitable = false;
	uint32_t selectedPhysicalDeviceID = 0;
	VkPhysicalDevice& selectedPhysicalDevice = physicalDevices[selectedPhysicalDeviceID];
	do
	{
		if (physicalDeviceCount > 1)
		{
			std::cout << " Pick vk physical device: ";
			do
			{
				std::cin >> selectedPhysicalDeviceID;
			} while (selectedPhysicalDeviceID > physicalDeviceCount);
		}

		selectedPhysicalDevice = physicalDevices[selectedPhysicalDeviceID];

		isSuitable = isDeviceSuitable(selectedPhysicalDevice);
		if (!isSuitable)
		{
			std::cerr << "ERROR: The selected Device is not suitable!\n";
		}

	} while (!isSuitable);

	if (!checkDeviceValidationLayersSupport(selectedPhysicalDevice, ReqLayers))
	{
		return;
	}

	if (!checkDeviceExtensionsSupport(selectedPhysicalDevice, ReqExt))
	{
		return;
	}

	auto queueFamilies = findQueueFamilies(selectedPhysicalDevice);
	auto queuePriority = 1.0f;

	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pNext = 0;
	queueCreateInfo.flags = 0;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.queueFamilyIndex = queueFamilies.graphicsFamily;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures physicalDeviceFeatures{};
	physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.pNext = 0;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(ReqExt.size());
	deviceCreateInfo.ppEnabledExtensionNames = ReqExt.data();
	deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ReqLayers.size());
	deviceCreateInfo.ppEnabledLayerNames = ReqLayers.data();
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

	physicalDevice = selectedPhysicalDevice;

	if (vkCreateDevice(selectedPhysicalDevice, &deviceCreateInfo, nullptr, device.replace()) != VK_SUCCESS)
		std::runtime_error("ERROR: Cannot create the logical device.\n");

	vkGetDeviceQueue(device, queueFamilies.graphicsFamily, 0, &graphicsQueue);
}

void HelloTriangleApp::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, surface.replace()) != VK_SUCCESS)
	{
		std::runtime_error("Failed to create window surface.\n");
	}
}

void HelloTriangleApp::createSwapChain()
{
	SwapChainSupportDeatils swapChainSupportDetails = querySwapChainSupportDetails(physicalDevice);

	const auto& surfaceFormat = chooseSwapChainSurfaceFormat(swapChainSupportDetails.formats);
	const auto& presentMode = chooseSwapChainPresentationMode(swapChainSupportDetails.presentModes);
	const auto& extent = chooseSwapChainExtent(swapChainSupportDetails.caps);

	uint32_t imageCount = swapChainSupportDetails.caps.minImageCount + 1;
	if (swapChainSupportDetails.caps.maxImageCount > 0 && swapChainSupportDetails.caps.maxImageCount < imageCount)
	{
		imageCount = swapChainSupportDetails.caps.maxImageCount;
	}

	auto oldSwapChain = swapChain;

	VkSwapchainCreateInfoKHR swapChainCreateInfo{};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.flags = 0;
	swapChainCreateInfo.surface = surface;
	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageFormat = surfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChainCreateInfo.presentMode = presentMode;
	swapChainCreateInfo.imageExtent = extent;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainCreateInfo.queueFamilyIndexCount = 0;
	swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	swapChainCreateInfo.preTransform = swapChainSupportDetails.caps.currentTransform;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.clipped = VK_TRUE;
	swapChainCreateInfo.oldSwapchain = oldSwapChain;
	
	VkSwapchainKHR newSwapChain = VK_NULL_HANDLE;
	if (vkCreateSwapchainKHR(device, &swapChainCreateInfo, nullptr, &newSwapChain) != VK_SUCCESS)
	{
		assert(false);
		std::runtime_error("ERROR: failed to create swap chain.\n");
		return;
	}
	swapChain = newSwapChain;

	uint32_t swapChainCreateImageCount = 0;
	vkGetSwapchainImagesKHR(device, swapChain, &swapChainCreateImageCount, nullptr);
	swapChainImages.resize(swapChainCreateImageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &swapChainCreateImageCount, swapChainImages.data());
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void HelloTriangleApp::createSwapChainImageViews()
{
	swapChainImageViews.resize(swapChainImages.size(), VkDeleter<VkImageView>(device, vkDestroyImageView));

	for (size_t i = 0; i < swapChainImageViews.size(); ++i)
	{
		VkHelper::create2DImageView(device, swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, swapChainImageViews[i].replace());
	}
}

void HelloTriangleApp::createRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.flags = 0;
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.flags = 0;
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::array<VkAttachmentDescription, 2> attachmentDescs = { colorAttachment, depthAttachment };

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	
	VkSubpassDependency subpassDependency{};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = VK_NULL_HANDLE;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
	renderPassCreateInfo.pAttachments = attachmentDescs.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &subpassDependency;
	if(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, renderPass.replace()) != VK_SUCCESS)
		std::runtime_error("Cannot create render pass!");
}

void HelloTriangleApp::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uniformBinding{};
	uniformBinding.binding = 0;
	uniformBinding.descriptorCount = 1;
	uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerBinding{};
	samplerBinding.binding = 1;
	samplerBinding.descriptorCount = 1;
	samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uniformBinding, samplerBinding };

	VkDescriptorSetLayoutCreateInfo modelViewProjDescSetLayoutCreateInfo{};
	modelViewProjDescSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	modelViewProjDescSetLayoutCreateInfo.pNext = VK_NULL_HANDLE;
	modelViewProjDescSetLayoutCreateInfo.flags = 0;
	modelViewProjDescSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	modelViewProjDescSetLayoutCreateInfo.pBindings = bindings.data();

	if(vkCreateDescriptorSetLayout(device, &modelViewProjDescSetLayoutCreateInfo, nullptr, descSetLayout.replace()) != VK_SUCCESS)
		std::runtime_error("ERROR: Failed to create descriptor layout.");

}

void HelloTriangleApp::createGraphicsPipeline()
{
	auto vertShaderCode = ShaderHelper::readFile("shaders\\spir-v\\simple\\vert.spv");
	auto fragShaderCode = ShaderHelper::readFile("shaders\\spir-v\\simple\\frag.spv");

	VkDeleter<VkShaderModule> vertShaderModule{ device, vkDestroyShaderModule };
	VkDeleter<VkShaderModule> fragShaderModule{ device, vkDestroyShaderModule };

	ShaderHelper::createShaderModule(device, vertShaderCode, vertShaderModule);
	ShaderHelper::createShaderModule(device, fragShaderCode, fragShaderModule);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.pNext = VK_NULL_HANDLE;
	vertShaderStageInfo.flags = 0;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	vertShaderStageInfo.pSpecializationInfo = VK_NULL_HANDLE;	

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.pNext = VK_NULL_HANDLE;
	fragShaderStageInfo.flags = 0;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = VK_NULL_HANDLE;

	VkPipelineShaderStageCreateInfo pipelineStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	auto bindingDesc = mesh->getBindingDescription();
	auto attribDesc = mesh->getAttribDescription();

	// Vertex Input
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.pNext = VK_NULL_HANDLE;
	vertexInputCreateInfo.flags = 0;
	vertexInputCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDesc.size());
	vertexInputCreateInfo.pVertexBindingDescriptions = bindingDesc.data();
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDesc.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions = attribDesc.data();

	// Input Assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.pNext = VK_NULL_HANDLE;
	inputAssemblyCreateInfo.flags = 0;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0,0 };
	scissor.extent = swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportCreateInfo{};
	viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportCreateInfo.pNext = VK_NULL_HANDLE;
	viewportCreateInfo.flags = 0;
	viewportCreateInfo.viewportCount = 1;
	viewportCreateInfo.pViewports = &viewport;
	viewportCreateInfo.scissorCount = 1;
	viewportCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.pNext = VK_NULL_HANDLE;
	rasterizerCreateInfo.flags = 0;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerCreateInfo.lineWidth = 1.0f;
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_NONE;
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
	rasterizerCreateInfo.depthBiasClamp = 0.0f;
	rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multiSamplingCreateInfo{};
	multiSamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multiSamplingCreateInfo.pNext = VK_NULL_HANDLE;
	multiSamplingCreateInfo.flags = 0;
	multiSamplingCreateInfo.sampleShadingEnable = VK_FALSE;
	multiSamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multiSamplingCreateInfo.minSampleShading = 1.0f;
	multiSamplingCreateInfo.pSampleMask = VK_NULL_HANDLE;
	multiSamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multiSamplingCreateInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.pNext = VK_NULL_HANDLE;
	depthStencilCreateInfo.flags = 0;
	depthStencilCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilCreateInfo.minDepthBounds = 0.0f;
	depthStencilCreateInfo.maxDepthBounds = 1.0f;
	depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState attachmentColorBlendingCreateInfo{};
	attachmentColorBlendingCreateInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	attachmentColorBlendingCreateInfo.blendEnable = VK_FALSE;
	attachmentColorBlendingCreateInfo.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	attachmentColorBlendingCreateInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	attachmentColorBlendingCreateInfo.colorBlendOp = VK_BLEND_OP_ADD;
	attachmentColorBlendingCreateInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	attachmentColorBlendingCreateInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	attachmentColorBlendingCreateInfo.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo{};
	colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendingCreateInfo.pNext = VK_NULL_HANDLE;
	colorBlendingCreateInfo.flags = 0;
	colorBlendingCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendingCreateInfo.attachmentCount = 1;
	colorBlendingCreateInfo.pAttachments = &attachmentColorBlendingCreateInfo;

	VkDynamicState dynamicStates[] =
	{
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.pNext = VK_NULL_HANDLE;
	dynamicStateCreateInfo.flags = 0;
	dynamicStateCreateInfo.dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState);
	dynamicStateCreateInfo.pDynamicStates = dynamicStates;

	VkDescriptorSetLayout setsLayout[] = { descSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = VK_NULL_HANDLE;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = setsLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = VK_NULL_HANDLE;
	
	if(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, pipelineLayout.replace()) != VK_SUCCESS)
		std::runtime_error("Cannot create the pipeline layout.");

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.pNext = VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.flags = 0;
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = pipelineStages;
	graphicsPipelineCreateInfo.layout = pipelineLayout;
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &viewportCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multiSamplingCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.subpass = 0;
	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex = -1;

	if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, pipeline.replace()) != VK_SUCCESS)
		std::runtime_error("Cannot create graphics pipeline.");
}

void HelloTriangleApp::createFramebuffers()
{
	swapChainFrameBuffers.resize(swapChainImageViews.size(), VkDeleter<VkFramebuffer>{device, vkDestroyFramebuffer});
	for (size_t i = 0; i < swapChainImageViews.size(); ++i)
	{
		VkImageView attachments[] = 
		{
			swapChainImageViews[i],
			depthImgView
		};

		VkFramebufferCreateInfo frameBufferCreateInfo{};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = VK_NULL_HANDLE;
		frameBufferCreateInfo.flags = 0;
		frameBufferCreateInfo.width = swapChainExtent.width;
		frameBufferCreateInfo.height = swapChainExtent.height;
		frameBufferCreateInfo.layers = 1;
		frameBufferCreateInfo.renderPass = renderPass;
		frameBufferCreateInfo.attachmentCount = sizeof(attachments) / sizeof(VkImageView);
		frameBufferCreateInfo.pAttachments = attachments;
		
		if (vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, swapChainFrameBuffers[i].replace()) != VK_SUCCESS)
		{
			std::runtime_error("Failed to create frame buffer.");
		}
	}
}

void HelloTriangleApp::createCommandPool()
{
	VkCommandPoolCreateInfo cmdPoolCreateInfo{};
	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.pNext = VK_NULL_HANDLE;
	cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	cmdPoolCreateInfo.queueFamilyIndex = findQueueFamilies(physicalDevice).graphicsFamily;

	if (vkCreateCommandPool(device, &cmdPoolCreateInfo, nullptr, cmdPool.replace()) != VK_SUCCESS)
	{
		std::runtime_error("Cannot create command pool.");
	}
}

void HelloTriangleApp::createDepthResources()
{
	depthFormat = findDepthFormat();
	VkHelper::create2DImage(
		device, physicalDevice,
		swapChainExtent.width, swapChainExtent.height, depthFormat,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depthImg.replace(), depthImgMemory.replace());
	VkHelper::create2DImageView(device, depthImg, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, depthImgView.replace());
	VkHelper::transitImageLayout(device, cmdPool, graphicsQueue, depthImg, depthFormat, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void HelloTriangleApp::createTextureImages()
{
	int texWidth, texHeight, texChannels;
	std::string filename = texturePath;
	stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4; // RGBA
	if (!pixels)
		std::runtime_error("ERROR: Failed to load the texture (" + filename + ")");

	VkDeleter<VkImage> stagingImg{ device, vkDestroyImage };
	VkDeleter<VkDeviceMemory> stagingImgMem{ device, vkFreeMemory };

	VkHelper::create2DImage(device, physicalDevice,
		texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		stagingImg.replace(), stagingImgMem.replace());

	void* data;
	if (vkMapMemory(device, stagingImgMem, 0, imageSize, 0, &data) != VK_SUCCESS)
		std::runtime_error("ERROR: Failed to map memory.");
	{
		VkImageSubresource imgSubRsrc{};
		imgSubRsrc.arrayLayer = 0;
		imgSubRsrc.mipLevel = 0;
		imgSubRsrc.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		VkSubresourceLayout subRsrcLayout;
		vkGetImageSubresourceLayout(device, stagingImg, &imgSubRsrc, &subRsrcLayout);

		if (subRsrcLayout.rowPitch == 4 * texWidth)
		{
			memcpy(data, pixels, imageSize);
		}
		else
		{
			uint8_t* data_bytes = reinterpret_cast<uint8_t*>(data);
			for (int y = 0; y < texHeight; ++y)
			{
				memcpy(&data_bytes[y * subRsrcLayout.rowPitch], &pixels[y * texWidth * 4], 4 * texWidth);
			}
		}
	}
	vkUnmapMemory(device, stagingImgMem);

	VkHelper::create2DImage(device, physicalDevice,
		texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		mesh->textureImg.replace(), mesh->textureImgMem.replace());

	VkHelper::transitImageLayout(device, cmdPool, graphicsQueue, stagingImg, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	VkHelper::transitImageLayout(device, cmdPool, graphicsQueue, mesh->textureImg, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VkHelper::copyImage(device, cmdPool, graphicsQueue, stagingImg, mesh->textureImg, texWidth, texHeight);
	VkHelper::transitImageLayout(device, cmdPool, graphicsQueue, mesh->textureImg, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	stbi_image_free(pixels);
}

void HelloTriangleApp::createTextureImageViews()
{
	VkHelper::create2DImageView(device, mesh->textureImg, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, mesh->textureImgView.replace());
}

void HelloTriangleApp::createTextureSamplers()
{
	VkSamplerCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.pNext = VK_NULL_HANDLE;
	createInfo.magFilter = VK_FILTER_LINEAR;
	createInfo.minFilter = VK_FILTER_LINEAR;
	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.anisotropyEnable = VK_TRUE;
	createInfo.maxAnisotropy = 16;
	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	createInfo.unnormalizedCoordinates = VK_FALSE;
	createInfo.compareEnable = VK_FALSE;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.mipLodBias = 0.0f;
	createInfo.minLod = 0.0f;
	createInfo.maxLod = 0.0f;

	if(vkCreateSampler(device, &createInfo, nullptr, linearTextureSampler.replace()) != VK_SUCCESS)
		throw std::runtime_error("ERROR: Cannot create texture sampler.");
}
	  

void HelloTriangleApp::createVertexBuffers()
{
	VkDeleter<VkBuffer> staggingBuffer{ device, vkDestroyBuffer };
	VkDeleter<VkDeviceMemory> staggingMemBuffer{ device, vkFreeMemory };

	VkDeviceSize bufferSize = mesh->getVertexCount() * mesh->getVertexStride();
	VkHelper::createBuffer(
		device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
		staggingBuffer.replace(), staggingMemBuffer.replace()
	);

	void* data = nullptr;
	if (vkMapMemory(device, staggingMemBuffer, 0, bufferSize, 0, &data) != VK_SUCCESS)
		std::runtime_error("ERROR: Failed to map the buffer's memory.");
	memcpy(data, mesh->getVertices(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(device, staggingMemBuffer);

	VkHelper::createBuffer(
		device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		mesh->vertexBufferId.replace(), mesh->vertexBufferMem.replace()
	);

	VkHelper::copyBuffer(device, cmdPool, graphicsQueue, staggingBuffer, mesh->vertexBufferId, bufferSize);
}

void HelloTriangleApp::createIndexBuffers()
{
	VkDeleter<VkBuffer> staggingBuffer{ device, vkDestroyBuffer };
	VkDeleter<VkDeviceMemory> staggingMemBuffer{ device, vkFreeMemory };

	VkDeviceSize bufferSize = mesh->getIndexCount() * mesh->getIndexStide();
	VkHelper::createBuffer(
		device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		staggingBuffer.replace(), staggingMemBuffer.replace()
	);

	void* data = nullptr;
	if (vkMapMemory(device, staggingMemBuffer, 0, bufferSize, 0, &data) != VK_SUCCESS)
		std::runtime_error("ERROR: Failed to map the buffer's memory.");
	memcpy(data, mesh->getIndices(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(device, staggingMemBuffer);

	VkHelper::createBuffer(
		device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		mesh->indexBufferId.replace(), mesh->indexBufferMem.replace()
	);

	VkHelper::copyBuffer(device, cmdPool, graphicsQueue, staggingBuffer, mesh->indexBufferId, bufferSize);
}

void HelloTriangleApp::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(ModelViewProjUBO);

	VkHelper::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		modelViewProjStaggingUBOId.replace(), modelViewProjStaggingUBOMem.replace()
	);

	VkHelper::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		modelViewProjUBOId.replace(), modelViewProjUBOMem.replace()
	);
}

void HelloTriangleApp::createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSize = {};
	poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize[0].descriptorCount = 1;
	poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo descPoolCreateInfo{};
	descPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolCreateInfo.pNext = VK_NULL_HANDLE;
	descPoolCreateInfo.flags = 0;
	descPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
	descPoolCreateInfo.pPoolSizes = poolSize.data();
	descPoolCreateInfo.maxSets = 1;

	if (vkCreateDescriptorPool(device, &descPoolCreateInfo, nullptr, descPool.replace()) != VK_SUCCESS)
		std::runtime_error("ERROR: Failed to create descriptor pool.");
}

void HelloTriangleApp::createDescriptorSets()
{
	VkDescriptorSetLayout setLayouts[] = { descSetLayout };
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = VK_NULL_HANDLE;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = setLayouts;
	allocInfo.descriptorPool = descPool;

	if(vkAllocateDescriptorSets(device, &allocInfo, &descSet) != VK_SUCCESS)
		std::runtime_error("ERROR: Failed to allocate descriptor set.");

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = modelViewProjUBOId;
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;

	VkDescriptorImageInfo imgInfo{};
	imgInfo.imageView = mesh->textureImgView;
	imgInfo.sampler = linearTextureSampler;
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	std::array<VkWriteDescriptorSet, 2> writeDescSet = {};
	writeDescSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescSet[0].pNext = VK_NULL_HANDLE;
	writeDescSet[0].dstBinding = 0;
	writeDescSet[0].dstSet = descSet;
	writeDescSet[0].dstArrayElement = 0;
	writeDescSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescSet[0].descriptorCount = 1;	
	writeDescSet[0].pBufferInfo = &bufferInfo;

	writeDescSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescSet[1].pNext = VK_NULL_HANDLE;
	writeDescSet[1].dstBinding = 1;
	writeDescSet[1].dstSet = descSet;
	writeDescSet[1].dstArrayElement = 0;
	writeDescSet[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescSet[1].descriptorCount = 1;
	writeDescSet[1].pImageInfo = &imgInfo;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescSet.size()), writeDescSet.data(), 0, nullptr);
}

void HelloTriangleApp::createCommandBuffers()
{
	if (cmdBuffers.size() > 0)
	{
		vkFreeCommandBuffers(device, cmdPool, static_cast<uint32_t>(cmdBuffers.size()), cmdBuffers.data());
	}

	cmdBuffers.resize(swapChainImageViews.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = VK_NULL_HANDLE;
	allocInfo.commandPool = cmdPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers.size());

	if (vkAllocateCommandBuffers(device, &allocInfo, cmdBuffers.data()) != VK_SUCCESS)
	{
		std::runtime_error("Cannot allocate command buffers.");
	}

	for (size_t c = 0; c < cmdBuffers.size(); ++c)
	{
		auto& cmdBuffer = cmdBuffers[c];
		auto& frameBuffer = swapChainFrameBuffers[c];

		VkCommandBufferBeginInfo cmdBufferBeginInfo{};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufferBeginInfo.pNext = VK_NULL_HANDLE;
		cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		cmdBufferBeginInfo.pInheritanceInfo = 0;

		vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo);

		std::array<VkClearValue, 2> clearValues;
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = VK_NULL_HANDLE;
		renderPassBeginInfo.framebuffer = frameBuffer;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.renderArea.offset = { 0,0 };
		renderPassBeginInfo.renderArea.extent = swapChainExtent;
		renderPassBeginInfo.pClearValues = clearValues.data();
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

		vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkRect2D scissor{};
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent = swapChainExtent;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.width = static_cast<float>(swapChainExtent.width);
		viewport.height = static_cast<float>(swapChainExtent.height);

		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

		VkDeviceSize offsets[] = { 0 };
		VkBuffer buffers[] = { mesh->vertexBufferId };
		VkDescriptorSet descSets[] = { descSet };
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descSets, 0, nullptr);
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offsets);
		vkCmdBindIndexBuffer(cmdBuffer, mesh->indexBufferId, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmdBuffer, static_cast<uint32_t>(mesh->getIndexCount()), 1, 0, 0, 0);

		vkCmdEndRenderPass(cmdBuffer);
		
		if(vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS)
			std::runtime_error("Failed to record the command buffer.");
	}
}

void HelloTriangleApp::createSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.pNext = VK_NULL_HANDLE;
	semaphoreInfo.flags = 0;

	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, imageAvailableSemaphore.replace()) != VK_SUCCESS) std::runtime_error("Cannot create semaphore.");
	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, renderDoneSemaphore.replace()) != VK_SUCCESS) std::runtime_error("Cannot create semaphore.");
}

void HelloTriangleApp::recreateSwapChain()
{
	vkDeviceWaitIdle(device);

	createSwapChain();
	createSwapChainImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createDepthResources();
	createFramebuffers();
	createCommandBuffers();
}

void HelloTriangleApp::updateUniformBuffers()
{
	static double startTime = glfwGetTime();
	float time = static_cast<float>(glfwGetTime() - startTime);

	ModelViewProjUBO ubo{};
	ubo.model = glm::rotate(glm::mat4(), time * glm::radians(90.0f), glm::vec3(0, 1, 0));
	ubo.view = glm::lookAt(glm::vec3(2, 2, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / static_cast<float>(swapChainExtent.height), 0.1f, 10.0f);
	ubo.proj[1][1] *= -1.0f; // GL and Vulkan y-axis are different


	VkDeviceSize bufferSize = static_cast<VkDeviceSize>(sizeof(ubo));
	void* data;
	if(vkMapMemory(device, modelViewProjStaggingUBOMem, 0, bufferSize, 0, &data) != VK_SUCCESS)
		std::runtime_error("ERROR: Cannot map the memory.");
	{
		memcpy(data, &ubo, sizeof(ubo));
	}
	vkUnmapMemory(device, modelViewProjStaggingUBOMem);
	VkHelper::copyBuffer(device, cmdPool, graphicsQueue, modelViewProjStaggingUBOId, modelViewProjUBOId, bufferSize);
}
 
SwapChainSupportDeatils HelloTriangleApp::querySwapChainSupportDetails(VkPhysicalDevice physicalDevice)
{
	SwapChainSupportDeatils details;

	uint32_t formatCount = 0;
	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	if (formatCount > 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
	}
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
	if (presentModeCount > 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
	}
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.caps);

	return details;
}

VkSurfaceFormatKHR HelloTriangleApp::chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_B8G8R8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& format : availableFormats)
	{
		if(format.format == VK_FORMAT_B8G8R8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return format;
	}

	// Not Expected to be here but in case appropriate one is not found, return the first one.
	return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApp::chooseSwapChainPresentationMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (auto& presentMode : availablePresentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			bestMode = presentMode;
			return bestMode;
		}
		else if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			bestMode = presentMode;
		}

	}

	return bestMode;
}

VkExtent2D HelloTriangleApp::chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& caps)
{
	if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return caps.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent{ width, height };
		actualExtent.width = std::max(caps.minImageExtent.width, std::min(caps.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(caps.minImageExtent.height, std::min(caps.maxImageExtent.height, actualExtent.height));
		return actualExtent;
	}
}

void HelloTriangleApp::loop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		updateUniformBuffers();
		drawFrame();
	}

	vkDeviceWaitIdle(device);

	glfwDestroyWindow(window);
	glfwTerminate();
}

void HelloTriangleApp::drawFrame()
{
	uint32_t imgIdx = 0;
	switch (vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<int64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imgIdx))
	{
	case VK_SUCCESS:
	case VK_SUBOPTIMAL_KHR:
		break;

	case VK_ERROR_OUT_OF_DATE_KHR:
		recreateSwapChain();
		return;

	default:
		std::runtime_error("Cannot acquire swap chain image.");
		break;
	}

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = VK_NULL_HANDLE;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderDoneSemaphore;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffers[imgIdx];
	submitInfo.pWaitDstStageMask = &waitStage;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		std::runtime_error("Queue submission failed.");
		return;
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = VK_NULL_HANDLE;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapChain;
	presentInfo.pImageIndices = &imgIdx;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderDoneSemaphore;
	switch (vkQueuePresentKHR(graphicsQueue, &presentInfo))
	{
	case VK_SUCCESS:
		break;

	case VK_ERROR_OUT_OF_DATE_KHR:
	case VK_SUBOPTIMAL_KHR:
		recreateSwapChain();
		return;

	default:
		std::runtime_error("swap chain failed to present image on the screen.");
		break;
	}
}

#include <unordered_map>
#include <glm/gtx/hash.hpp>

namespace std
{
	template<> struct hash<Mesh3D::Vertex>
	{
		size_t operator()(const Mesh3D::Vertex& vertex) const
		{
			return 
				((std::hash<glm::vec3>()(vertex.pos) ^
				(std::hash<glm::vec3>()(vertex.color) << 1)) >> 1)  ^
				(std::hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
};

void HelloTriangleApp::loadMesh()
{
	//mesh = std::make_unique<ScreenSpaceMesh>(device);
	mesh = std::make_unique<Mesh3D>(device);

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, modelPath.c_str()))
	{
		throw std::runtime_error(err);
	}

	std::vector<Mesh3D::Vertex> vertices;
	std::vector<uint32_t> indices;

#if 1
	auto nVertices = attrib.vertices.size() / 3;
	auto nTexCoords = attrib.texcoords.size() / 2;

	assert(nVertices * 3 == attrib.vertices.size() && "The number of vertices is not divisible by 3.");
	assert(nTexCoords * 2 == attrib.texcoords.size() && "The number of texture coordinates is not even.");
	assert(nVertices == nTexCoords && "The number of vertices and texture coordinates are not the same.");

	std::map<std::pair<uint32_t, uint32_t>, uint32_t> vertexIndexMap;
	for (const auto& shape : shapes)
	{
		for (const auto& idx : shape.mesh.indices)
		{
			auto vIdx = idx.vertex_index;
			auto tIdx = idx.texcoord_index;
			auto pair = std::make_pair(vIdx, tIdx);
			if (vertexIndexMap.find(pair) == vertexIndexMap.end())
			{
				Mesh3D::Vertex vertex;

				vertex.pos.x = attrib.vertices[3 * vIdx + 0];
				vertex.pos.z = attrib.vertices[3 * vIdx + 1];
				vertex.pos.y = attrib.vertices[3 * vIdx + 2];

				vertex.texCoord.x = attrib.texcoords[2 * tIdx + 0];
				vertex.texCoord.y = 1.0f - attrib.texcoords[2 * tIdx + 1];

				vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);

				vertexIndexMap[pair] = vertices.size();
				vertices.push_back(vertex);
			}

			indices.push_back(vertexIndexMap[pair]);
		}
	}
#else
	std::unordered_map<Mesh3D::Vertex, int> vertexIndexMap;

	for (const auto& shape : shapes)
	{
		for (const auto& idx : shape.mesh.indices)
		{
			Mesh3D::Vertex vertex;

			vertex.pos.x = attrib.vertices[3 * idx.vertex_index + 0];
			vertex.pos.z = attrib.vertices[3 * idx.vertex_index + 1];
			vertex.pos.y = attrib.vertices[3 * idx.vertex_index + 2];

			vertex.texCoord.x = attrib.texcoords[2 * idx.texcoord_index + 0];
			vertex.texCoord.y = 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1];

			vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);

			if (vertexIndexMap.count(vertex) == 0)
			{
				vertexIndexMap[vertex] = vertices.size();
				vertices.push_back(vertex);
			}	

			indices.push_back(vertexIndexMap[vertex]);
		}
	}
#endif

	std::cout	<< "======================\n"
				<< "== Mesh Info:\n"
				<< "======================\n"
				<< " Vertex Count " << vertices.size() << "\n"
				<< " Index Count " << indices.size() << "\n"
				<< "======================\n";

	(static_cast<Mesh3D*>(mesh.get()))->setVertices(vertices);
	(static_cast<Mesh3D*>(mesh.get()))->setIndices(indices);
}