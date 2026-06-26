#include "Game/Game.h"
#include "Core/ImGuiHandler.h"
#include "Core/Utils.h"
#include "Core/ImGuiUtils.h"
#include "Game/Noise.h"

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
	m_UICamera->view_box.width = width;
	m_UICamera->view_box.height = height;
	m_UICamera->UpdateProjection();
	glViewport(0, 0, width, height);
	m_MainFrameBuffer->Resize(width, height);
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

	// Create Main Camera
	Frustum frustum;
	frustum.aspect_ratio = static_cast<float>(m_Window->GetWidth()) / static_cast<float>(m_Window->GetHeight());
	frustum.fov = glm::radians(70.0f);
	frustum.near = 0.1f;
	frustum.far = 1000.0f;
	m_Camera = CreateRef<Camera>(frustum, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_Camera->eular.y = PIHalf;

	// Create UI Camera
	ViewBox view_box;
	view_box.width = m_Window->GetWidth();
	view_box.height = m_Window->GetHeight();
	view_box.near = 0.1f;
	view_box.far = 1000.0f;
	m_UICamera = CreateRef<OrthographicCamera>(view_box, glm::vec3(0.0f));

	// Create Shaders
	m_PostProcShader = CreateRef<Shader>("assets/shaders/PostProc.vert", "assets/shaders/PostProc.frag");
	m_DebugShader = CreateRef<Shader>("assets/shaders/DebugLine.vert", "assets/shaders/DebugLine.frag");
	m_UIShader = CreateRef<Shader>("assets/shaders/UI.vert", "assets/shaders/UI.frag");

	// Create Main Frame Buffer
	m_MainFrameBuffer = CreateRef<FrameBuffer>(m_Window->GetWidth(), m_Window->GetHeight());

	// Create UI Quad
	m_UIQuad = CreateRef<VertexArray>();
	m_UIQuad->Bind();

	VertexLayout vertex_layout = { {
		{ GL_FLOAT, 2 },
		{ GL_FLOAT, 2 }
	} };
	Ref<VertexBuffer> vertex_buffer = CreateRef<VertexBuffer>(c_UIQuadVertices, 16 * sizeof(float), vertex_layout);
	m_UIQuad->GetVertexBuffer() = vertex_buffer;

	Ref<IndexBuffer> index_buffer = CreateRef<IndexBuffer>(c_UIQuadIndices, 6 * sizeof(uint32_t));
	m_UIQuad->GetIndexBuffer() = index_buffer;

	m_UIQuad->Unbind();

	ImGuiHandler::Init(m_Window);

	// Other open gl configs
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(3.0f);
	glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
	glClearColor(0.125, 0.13, 0.2, 1);

	NewWorld();
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

	// Render Scene
	m_MainFrameBuffer->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_Player->Render();
	m_World->Render(m_Camera);

	m_MainFrameBuffer->Unbind();

	m_PostProcShader->Bind();
	m_MainFrameBuffer->GetColorBuffer()->Bind();
	m_PostProcShader->SetUniform("u_FrameTexture", m_MainFrameBuffer->GetColorBuffer());
	m_UIQuad->Bind();
	glDrawElements(GL_TRIANGLES, m_UIQuad->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	m_UIQuad->Unbind();
	m_MainFrameBuffer->GetColorBuffer()->Unbind();
	m_PostProcShader->Unbind();

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
	if (ImGui::CollapsingHeader("Noise")) {
		
		ImGui::InputInt("Seed", &m_NoiseSeed);
		ImGui::InputInt("Width", &m_NoiseWidth);
		ImGui::InputInt("Height", &m_NoiseHeight);
		ImGui::InputInt("Frequency", &m_NoiseFrequency);
		ImGui::InputFloat2("Offset", &m_NoiseOffset.x);
		if (ImGui::Button("White Noise")) {
			m_Noise = NoiseGenerator::GenerateWhiteNoise(m_NoiseWidth, m_NoiseHeight, (uint32_t)glfwGetTime());
		}
		if (ImGui::Button("Perlin Noise")) {
			m_Noise = NoiseGenerator::GeneratePerlinNoise(m_NoiseWidth, m_NoiseHeight, m_NoiseFrequency, m_NoiseOffset, m_NoiseSeed);
		}

		if (m_Noise != nullptr) {
			ImGuiImage("Noise", m_Noise, { 0.0f, 300.0f });
		}
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

void Game::NewWorld() {
	m_World = CreateRef<World>(6437065572);
	// Spawn player on the ground
	glm::vec3 player_position = glm::vec3(0.0f, Chunk::ChunkHeight - 1.0f, 0.0f);
	player_position = m_World->CastRay(Ray(player_position, glm::vec3(0.0f, -1.0f, 0.0f))).voxel_position;
	player_position += glm::vec3(0.0f, 1.001f, 0.0f);
	m_Player = CreateRef<Player>(player_position, m_Camera, m_World);
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