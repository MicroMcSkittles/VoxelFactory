#include "Core/Utils.h"

#include <iostream>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

void GLFWErrorCallback(int code, const char* desc) {
	std::cerr << "glfw error has occured ( " << code << " ): " << desc << std::endl;
}

int main(int argc, char** argv) {

	// Init glfw
	glfwSetErrorCallback(GLFWErrorCallback);
	ASSERT(glfwInit());

	// Create window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window_handle = glfwCreateWindow(800, 600, "Voxel Factory", nullptr, nullptr);
	ASSERT(window_handle);

	// Init glad
	glfwMakeContextCurrent(window_handle);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glViewport(0, 0, 800, 600);
	glClearColor(1, 0, 1, 1);

	// Main loop
	while (!glfwWindowShouldClose(window_handle)) {
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window_handle);
		glfwPollEvents();
	}

	// Clean up
	glfwDestroyWindow(window_handle);
	glfwTerminate();

	return 0;
}