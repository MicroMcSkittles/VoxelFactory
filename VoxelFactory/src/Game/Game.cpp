#include "Game/Game.h"
#include "Core/ImGuiHandler.h"
#include "Core/Utils.h"

#include <iostream>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include "Player.h"

Game::Game() {
	m_Running = true;

	m_Focused = false;
	m_MouseAvalible = true;
	m_LastMousePos = glm::vec2(0.0f);

	m_DebugLineMesh = nullptr;
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
void Game::OnMouseClick(int button, int action, int mods) {
	if (action != GLFW_PRESS) return;
	if (button == GLFW_MOUSE_BUTTON_LEFT) OnLeftClick();
	else if (button == GLFW_MOUSE_BUTTON_RIGHT) OnRightClick();
}
void Game::OnLeftClick() {
	GLFWwindow* window_handle = (GLFWwindow*)m_Window->GetHandle();

	// If window not focused and mouse isnt hovering gui than capture cursor
	if (!m_Focused && m_MouseAvalible) {
		m_Focused = true;

		double mouse_x = 0.0, mouse_y = 0.0;
		glfwGetCursorPos(window_handle, &mouse_x, &mouse_y);
		m_LastMousePos = glm::vec2(static_cast<float>(mouse_x), static_cast<float>(mouse_y));
		glfwSetInputMode(window_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
		return;
	}
	else if (!m_Focused) return;

	m_Player->OnLeftClick();
}
void Game::OnRightClick() {
	m_Player->OnRightClick();
}

void Game::StartUp() {
	// Create window and set window events
	m_Window = CreateRef<Window>(1080, 800, "Voxel Factory");
	GLFWwindow* window_handle = (GLFWwindow*)m_Window->GetHandle();
	glfwSetWindowUserPointer(window_handle, this);
	glfwSetWindowSizeCallback(window_handle, [](GLFWwindow* window, int width, int height) {
		Game* game = reinterpret_cast<Game*>(glfwGetWindowUserPointer(window));
		game->OnResize(width, height);
	});
	glfwSetMouseButtonCallback(window_handle, [](GLFWwindow* window, int button, int action, int mods) {
		Game* game = reinterpret_cast<Game*>(glfwGetWindowUserPointer(window));
		game->OnMouseClick(button, action, mods);
	});

	m_World = CreateRef<World>();

	// Create camera
	Frustum frustum;
	frustum.aspect_ratio = static_cast<float>(m_Window->GetWidth()) / static_cast<float>(m_Window->GetHeight());
	frustum.fov = PI / 4.0f; // 45 degrees
	frustum.near = 0.1f;
	frustum.far = 1000.0f;
	m_Camera = CreateRef<Camera>(frustum, glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_Camera->eular.y = PIHalf;

	// Other open gl configs
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(3.0f);
	glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
	glClearColor(0.125, 0.13, 0.2, 1);

	ImGuiHandler::Init(m_Window);

	m_DebugShader = CreateRef<Shader>("assets/shaders/DebugLine.vert", "assets/shaders/DebugLine.frag");

	m_Player = CreateRef<Player>(m_Camera, m_World);
}
void Game::Update(float delta_time) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ImGuiHandler::StartFrame();

	GLFWwindow* window_handle = (GLFWwindow*)m_Window->GetHandle();
	
	// Release mouse on escape
	if (glfwGetKey(window_handle, GLFW_KEY_ESCAPE)) {
		m_Focused = false;
		glfwSetInputMode(window_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	if (m_Focused) {
		m_Player->Update(delta_time, m_LastMousePos, m_Window);
	}

	m_World->Update(m_Camera->position);

	m_Player->Render();
	m_World->Render(m_Camera);

	ShowDebugLines();

	ShowImGui();
	ImGuiHandler::EndFrame();
}
void Game::ShowImGui() {

	ImGui::Begin("Debug Menu");
	ImVec2 window_pos = ImGui::GetWindowPos();
	
	if (ImGui::Button("Clear Debug Lines")) {
		ClearDebugLines();
	}
	
	if (ImGui::CollapsingHeader("Player")) {
		m_Player->ShowImGui();
	}
	if (ImGui::CollapsingHeader("World")) {
		m_World->ShowImGui();
	}

	ImVec2 window_size = ImGui::GetWindowSize();
	window_size.x += window_pos.x;
	window_size.y += window_pos.y;
	ImGui::End();
	ImVec2 cursor_pos = ImGui::GetMousePos();
	
	m_MouseAvalible = (window_pos.x > cursor_pos.x || window_size.x < cursor_pos.x) || (window_pos.y > cursor_pos.y || window_size.y < cursor_pos.y);
	m_MouseAvalible &= !m_Focused;
}
void Game::ShutDown() {

}

void Game::ClearDebugLines() {
	m_DebugLines.clear();
	m_DebugLineMesh = nullptr;
}
void Game::PushDebugLine(const DebugLine& line) {
	m_DebugLines.push_back(line);
}
void Game::ShowDebugLines() {
	if (m_DebugLines.empty()) return;
	m_DebugLineMesh = CreateRef<VertexArray>();
	m_DebugLineMesh->Bind();

	VertexLayout vertex_layout = { {
		{ GL_FLOAT, 3 },
		{ GL_FLOAT, 3 }
	} };
	Ref<VertexBuffer> vertex_buffer = CreateRef<VertexBuffer>(m_DebugLines.data(), m_DebugLines.size() * sizeof(DebugLine), vertex_layout);
	vertex_buffer->Bind();
	m_DebugLineMesh->GetVertexBuffer() = vertex_buffer;

	m_DebugShader->Bind();
	m_DebugShader->SetUniform("u_ViewProjection", m_Camera->view_projection);

	glDrawArrays(GL_LINES, 0, 2 * m_DebugLines.size());

	m_DebugShader->Unbind();
	m_DebugLineMesh->Unbind();
}