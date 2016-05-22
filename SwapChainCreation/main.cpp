#include "Renderer.h"

#include "Helper.h"

int main() {

	std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>();

	renderer->createWindow(800, 600, "Test");

	while (renderer->run()) {

	}

	return 0;
}