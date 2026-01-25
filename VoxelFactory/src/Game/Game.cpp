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

Game::Game() {
	m_Running = true;
	m_Model = glm::mat4(1.0f);

	m_Focused = false;
	m_CameraSpeed = 4.0f;
	m_MouseSensitivity = 0.1f;
	m_LastMousePos = glm::vec2(0.0f);
}
Game::~Game() { }

void Game::Run() {
	float last_ms = glfwGetTime();
	StartUp();

	// Main loop
	while (m_Running && !m_Window->ShouldClose()) {
		
		// Calculate delta time
		float current_ms = glfwGetTime();
		float delta_time = current_ms - last_ms;
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

void Game::UpdateCamera(float delta_time) {
	GLFWwindow* window_handle = (GLFWwindow*)m_Window->GetHandle();
	
	// Capture mouse on left click, release on escape
	if (!m_Focused && glfwGetMouseButton(window_handle, GLFW_MOUSE_BUTTON_LEFT)) {
		m_Focused = true;
		double mouse_x = 0.0, mouse_y = 0.0;
		glfwGetCursorPos(window_handle, &mouse_x, &mouse_y);
		m_LastMousePos = glm::vec2(static_cast<float>(mouse_x), static_cast<float>(mouse_y));
		glfwSetInputMode(window_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else if(!m_Focused) return;
	if (glfwGetKey(window_handle, GLFW_KEY_ESCAPE)) {
		m_Focused = false;
		glfwSetInputMode(window_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		return;
	}
	
	// Camera position input
	glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 camera_right = glm::normalize(glm::cross(m_Camera->direction, camera_up));
	glm::vec3 delta_position = glm::vec3(0.0f);
	if (glfwGetKey(window_handle, GLFW_KEY_W))          delta_position += m_Camera->direction * m_CameraSpeed * delta_time;
	if (glfwGetKey(window_handle, GLFW_KEY_S))          delta_position -= m_Camera->direction * m_CameraSpeed * delta_time;
	if (glfwGetKey(window_handle, GLFW_KEY_A))          delta_position -= camera_right * m_CameraSpeed * delta_time;
	if (glfwGetKey(window_handle, GLFW_KEY_D))          delta_position += camera_right * m_CameraSpeed * delta_time;
	if (glfwGetKey(window_handle, GLFW_KEY_SPACE))      delta_position += camera_up * m_CameraSpeed * delta_time;
	if (glfwGetKey(window_handle, GLFW_KEY_LEFT_SHIFT)) delta_position -= camera_up * m_CameraSpeed * delta_time;

	// Get mouse delta
	double mouse_x = 0.0, mouse_y = 0.0;
	glfwGetCursorPos(window_handle, &mouse_x, &mouse_y);
	glm::vec2 mouse_pos = glm::vec2(static_cast<float>(mouse_x), static_cast<float>(mouse_y));
	glm::vec2 delta_mouse = {
		mouse_pos.x - m_LastMousePos.x,
		m_LastMousePos.y - mouse_pos.y // Invert Y, because opengl is bottom to top
	};
	delta_mouse *= ((m_MouseSensitivity * PI) / 180.0f);

	m_LastMousePos = mouse_pos;

	// Adjust rotation
	m_Camera->eular.y += delta_mouse.x; // Yaw
	m_Camera->eular.x += delta_mouse.y; // Pitch

	// Cap pitch
	constexpr float c_MaxPitch =  PIHalf - (PI / 180.0f); // 89 degrees
	constexpr float c_MinPitch = -PIHalf + (PI / 180.0f); // -89 degrees
	m_Camera->eular.x = std::min(c_MaxPitch, std::max(m_Camera->eular.x, c_MinPitch));

	if (delta_position != glm::vec3(0.0f) || delta_mouse != glm::vec2(0.0f)) {
		m_Camera->direction = Camera::EulerDirection(m_Camera->eular.x, m_Camera->eular.y);
		m_Camera->position += delta_position;
		m_Camera->UpdateView();
	}
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
	m_Camera->eular.y = PIHalf;

	// Other configs
	glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
	glClearColor(0.125, 0.13, 0.2, 1);
}
void Game::Update(float delta_time) {
	glClear(GL_COLOR_BUFFER_BIT);

	UpdateCamera(delta_time);

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
