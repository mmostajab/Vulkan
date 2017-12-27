// HelloTriangle.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "HelloTriangleApp.h"

int main()
{
	HelloTriangleApp app;

	try
	{
		app.run();
	}
	catch (const std::runtime_error& e) 
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

