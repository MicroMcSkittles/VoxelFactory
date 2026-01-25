#include "Game/Game.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const glm::vec3 c_Vertices[] = {
	{ -0.5f, -0.5f, 0.0f },
	{  0.5f, -0.5f, 0.0f },
	{  0.0f,  0.5f, 0.0f }
};
const uint32_t c_Indices[] = {
	0, 1, 2
};

Game::Game(): m_Running(true), m_Model(1.0f), m_Time(0.0f) { }
Game::~Game() { }

void Game::Run() {
	double last_ms = glfwGetTime();
	StartUp();

	// Main loop
	while (m_Running && !m_Window->ShouldClose()) {
		
		// Calculate delta time
		double current_ms = glfwGetTime();
		double delta_time = current_ms - last_ms;
		last_ms = current_ms;

		Update(delta_time);
		m_Window->Update();
	}
	ShutDown();
}

void Game::OnResize(int width, int height) {
	m_Camera->frustum.aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	m_Camera->UpdateProjection();
	glViewport(0, 0, width, height);
}

void Game::StartUp() {
	// Create window and set window events
	m_Window = CreateRef<Window>(1080, 800, "Voxel Factory");
	glfwSetWindowUserPointer((GLFWwindow*)m_Window->GetHandle(), this);
	glfwSetWindowSizeCallback((GLFWwindow*)m_Window->GetHandle(), [](GLFWwindow* window, int width, int height) {
		Game* game = reinterpret_cast<Game*>(glfwGetWindowUserPointer(window));
		game->OnResize(width, height);
	});

	m_MainShader = CreateRef<Shader>("assets/shaders/Main.vert", "assets/shaders/Main.frag");

	// Create triangle
	m_VAO = CreateRef<VertexArray>();
	m_VAO->Bind();

	Ref<VertexBuffer> vertex_buffer = CreateRef<VertexBuffer>(c_Vertices, sizeof(c_Vertices));
	vertex_buffer->Bind();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	Ref<IndexBuffer> index_buffer = CreateRef<IndexBuffer>(c_Indices, sizeof(c_Indices));
	m_VAO->Unbind();
	m_Model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 3.0f));

	// Create camera
	Frustum frustum;
	frustum.aspect_ratio = static_cast<float>(m_Window->GetWidth()) / static_cast<float>(m_Window->GetHeight());
	frustum.fov = PI / 4.0f; // 45 degrees
	frustum.near = 0.1f;
	frustum.far = 1000.0f;
	m_Camera = CreateRef<Camera>(frustum, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	// Other configs
	glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
	glClearColor(0.125, 0.13, 0.2, 1);
}
void Game::Update(double delta_time) {
	glClear(GL_COLOR_BUFFER_BIT);

	m_Time += delta_time;
	m_Camera->position.x = cos(m_Time);
	m_Camera->position.z = sin(m_Time);
	m_Camera->UpdateView();

	m_MainShader->Bind();
	m_MainShader->SetUniform("u_ViewProjection", m_Camera->view_projection);
	m_MainShader->SetUniform("u_Model", m_Model);
	m_VAO->Bind();
	glDrawElements(GL_TRIANGLES, sizeof(c_Indices) / sizeof(uint32_t), GL_UNSIGNED_INT, nullptr);
	m_VAO->Unbind();
	m_MainShader->Unbind();
}
void Game::ShutDown() {

}
