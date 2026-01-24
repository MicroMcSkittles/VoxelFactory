#include "Core/Window.h"
#include "Core/Utils.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

bool   Window::s_GLFWInitialized = false;
size_t Window::s_GLFWWindowCount = 0;

void Window::ErrorCallback(int error_code, const char* description) {
	std::cerr << "A glfw error has occured ( " << error_code << " ): " << description;
}

Window::Window(uint32_t width, uint32_t height, const std::string& title) : m_Width(width), m_Height(height), m_Handle(nullptr)
{
	// Init glfw
	if (!s_GLFWInitialized) {
		glfwSetErrorCallback(Window::ErrorCallback);
		ASSERT_MSG(glfwInit(), "Failed to initialize glfw");
		s_GLFWInitialized = true;
	}

	// Create window handle
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	m_Handle = glfwCreateWindow(800, 600, title.c_str(), nullptr, nullptr);
	ASSERT_MSG(m_Handle, "Failed to create window");

	// Init glad
	glfwMakeContextCurrent((GLFWwindow*)m_Handle);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	s_GLFWWindowCount += 1;
}

Window::~Window() {
	glfwDestroyWindow((GLFWwindow*)m_Handle);
	if (--s_GLFWWindowCount == 0) {
		glfwTerminate();
	}
}

void Window::Update() {
	glfwSwapBuffers((GLFWwindow*)m_Handle);
	glfwPollEvents();
}
void Window::MakeCurrent() {
	glfwMakeContextCurrent((GLFWwindow*)m_Handle);
}
bool Window::ShouldClose() {
	return glfwWindowShouldClose((GLFWwindow*)m_Handle);
}