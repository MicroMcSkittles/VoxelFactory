#include "Core/Core.h"
#include "Core/Utils.h"
#include "Core/Window.h"

#include <iostream>
#include <glad/glad.h>

int main(int argc, char** argv) {

	Ref<Window> window = CreateRef<Window>(800, 600, "Voxel Factory");

	glViewport(0, 0, 800, 600);
	glClearColor(1, 0, 1, 1);

	// Main loop
	while (!window->ShouldClose()) {
		glClear(GL_COLOR_BUFFER_BIT);

		window->Update();
	}

	return 0;
}