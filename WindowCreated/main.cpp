#include "Renderer.h"

#include "Helper.h"

int main() {

	Renderer renderer;

	renderer.createWindow(800, 600, "Test");

	while (renderer.run()) {

	}

	return 0;
}