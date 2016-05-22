#include "Renderer.h"

#include "Helper.h"

// STD
#include <vector>

int main() {

	std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>();

	renderer->createWindow(800, 600, "Test");

	// Create the required resources for rendering a triangle.

	

	while (renderer->run()) {

	}

	return 0;
}