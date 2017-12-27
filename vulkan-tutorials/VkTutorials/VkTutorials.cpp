// VkTutorials.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main()
{
	glfwInit();
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//std::unique_ptr<GLFWwindow> window = std::unique_ptr<GLFWwindow>(glfwCreateWindow(windowRect.width, windowRect.height, "VkTutorials", nullptr, nullptr));
	GLFWwindow* window = glfwCreateWindow(windowRect.width, windowRect.height, "VkTutorials", nullptr, nullptr);

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "==============================================================\n";
	std::cout << "  Available extension count = " << extensionCount << std::endl;
	std::cout << "==============================================================\n";
	for (auto& ext : extensions) {
		std::cout << ext.extensionName << "(" << ext.specVersion << ")" << std::endl;
	}
	std::cout << "==============================================================\n";

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

    return 0;
}

