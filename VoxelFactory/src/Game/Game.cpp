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
	ASSERT(s_Instance == nullptr);
	s_Instance = this;

	m_State = GameState::InGame;
	m_Running = true;
	m_ShowStats = false;

	m_MouseCaptured = false;
	m_MouseAvalible = true;
	m_LastMousePos = glm::vec2(0.0f);

	m_FPS = 0.0f;
	m_DebugLineMesh = nullptr;
	m_ActiveMenu = MenuType::None;
}
Game::~Game() { }

void Game::Pause() {
	if (m_State != GameState::InGame) return;
	m_State = GameState::Paused;
	m_ActiveMenu = MenuType::Pause;
	ReleaseMouse();
}
void Game::Resume() {
	if (m_State != GameState::Paused) return;
	m_State = GameState::InGame;
	m_ActiveMenu = MenuType::None;
	CaptureMouse();
}
void Game::Quit() {
	m_Running = false;
	glfwSetWindowShouldClose((GLFWwindow*)m_Window->GetHandle(), true);
}
void Game::Run() {
	float last_ms = glfwGetTime();
	StartUp();

	// Main loop
	while (m_Running && !m_Window->ShouldClose()) {
		
		// Calculate delta time
		float current_ms = glfwGetTime();
		float delta_time = current_ms - last_ms;
		last_ms = current_ms;

		m_FPS = 1.0f / delta_time;

		ImGuiHandler::StartFrame();
		Update(delta_time);
		Render();
		ShowImGui();
		ImGuiHandler::EndFrame();

		m_Window->Update();
	}
	ShutDown();
}

void Game::OnResize(int width, int height) {
	if (width == 0 && height == 0) return;
	m_Camera->frustum.aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	m_Camera->UpdateProjection();

	glViewport(0, 0, width, height);
	m_MainFrameBuffer->Resize(width, height);
	
	m_UI->Resize(width, height);
}
void Game::OnMouseClick(int button, int action, int mods) {
	if (action != GLFW_PRESS) return;
	if (button == GLFW_MOUSE_BUTTON_LEFT) OnLeftClick();
	else if (button == GLFW_MOUSE_BUTTON_RIGHT) OnRightClick();
}
void Game::OnLeftClick() {
	GLFWwindow* window_handle = (GLFWwindow*)m_Window->GetHandle();

	// If window not focused and mouse isnt hovering gui than capture cursor
	if (!m_MouseCaptured && m_MouseAvalible && m_State == GameState::InGame) {
		CaptureMouse();
		return;
	}
	m_Player->OnLeftClick();
}
void Game::OnRightClick() {
	m_Player->OnRightClick();
}
void Game::OnScroll(float delta) {
	if (m_MouseCaptured && m_State == GameState::InGame) m_Player->OnScroll(delta);
}
void Game::OnKey(int key, int action, int mods) {
	if (m_Player->OnKey(key, action, mods)) return;
	if (key == GLFW_KEY_F3 && action == GLFW_PRESS) m_ShowStats = !m_ShowStats;
	else if (key == GLFW_KEY_F1 && action == GLFW_PRESS && m_MouseCaptured) ReleaseMouse();
	// Handle Escape Key
	else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		if (m_State == GameState::Paused) Resume();
		else if (m_State == GameState::InGame) Pause();
	}
}

void Game::CaptureMouse() {
	m_MouseCaptured = true;

	GLFWwindow* window_handle = (GLFWwindow*)m_Window->GetHandle();

	double mouse_x = 0.0, mouse_y = 0.0;
	glfwGetCursorPos(window_handle, &mouse_x, &mouse_y);
	m_LastMousePos = glm::vec2(static_cast<float>(mouse_x), static_cast<float>(mouse_y));
	glfwSetInputMode(window_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
void Game::ReleaseMouse() {
	m_MouseCaptured = false;
	GLFWwindow* window_handle = (GLFWwindow*)m_Window->GetHandle();
	glfwSetInputMode(window_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
	glfwSetScrollCallback(window_handle, [](GLFWwindow* window, double delta_x, double delta_y) {
		Game* game = reinterpret_cast<Game*>(glfwGetWindowUserPointer(window));
		game->OnScroll(delta_y);
	});
	glfwSetKeyCallback(window_handle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		Game* game = reinterpret_cast<Game*>(glfwGetWindowUserPointer(window));
		game->OnKey(key, action, mods);
	});

	// Create Main Camera
	Frustum frustum;
	frustum.aspect_ratio = static_cast<float>(m_Window->GetWidth()) / static_cast<float>(m_Window->GetHeight());
	frustum.fov = glm::radians(70.0f);
	frustum.near = 0.1f;
	frustum.far = 1000.0f;
	m_Camera = CreateRef<Camera>(frustum, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_Camera->eular.y = PIHalf;

	// Create Frame Buffer
	m_MainFrameBuffer = CreateRef<FrameBuffer>(m_Window->GetWidth(), m_Window->GetHeight(), GL_RGB, GL_RGB);

	ImGuiHandler::Init(m_Window);

	// Other open gl configs
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(3.0f);
	glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	m_UI = CreateRef<UI>(m_Window->GetWidth(), m_Window->GetHeight());

	m_Font = CreateRef<Font>("assets/font/Font.fnt");

	LoadShaders();
	LoadTextures();
	InitMenus();
	NewWorld(643706557);
}
void Game::Update(float delta_time) {
	GLFWwindow* window_handle = (GLFWwindow*)m_Window->GetHandle();
	
	if (m_State == GameState::InGame || m_State == GameState::Menu) {
		m_Player->Update(delta_time, m_LastMousePos, m_Window);
		m_World->Update(delta_time, m_Player);
	}
	else if (m_State == GameState::Paused) {
		m_Menus[(size_t)m_ActiveMenu]->Update();
	}
}
void Game::Render() {
	// Render Scene
	m_MainFrameBuffer->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (m_State == GameState::InGame || m_State == GameState::Paused || m_State == GameState::Menu) {
		m_World->RenderSkyBox(m_Camera);
		m_Player->Render();
		m_World->RenderWorld(m_Camera);
		m_World->RenderEntities(m_Camera);
	}

	m_MainFrameBuffer->Unbind();

	m_UI->StartFrame();
	// Show scene frame
	UI::TexturedQuad(glm::vec3(0.0f, 0.0f, -50.0f), UI::GetScreenMax(), m_MainFrameBuffer->GetColorBuffer());
	
	// Render UI
	if (m_State == GameState::InGame || m_State == GameState::Menu) {
		m_Player->RenderUI();
		if (m_ShowStats) ShowStatsOverlay();
	}
	else if (m_State == GameState::Paused) {
		m_Menus[(size_t)m_ActiveMenu]->Render();
	}
	m_UI->EndFrame();

	// Render Scene and UI layers
	Ref<Shader>& post_proc_shader = m_Shaders[(size_t)ShaderType::PostProc];
	post_proc_shader->Bind();

	Ref<VertexArray>& quad = m_UI->GetQuad();
	quad->Bind();

	// TODO: a layer system?
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render UI Frame
	Ref<Texture>& ui_frame = m_UI->GetFrame();
	ui_frame->Bind();
	post_proc_shader->SetUniform("u_FrameTexture", ui_frame);
	glDrawElements(GL_TRIANGLES, quad->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	ui_frame->Unbind();

	quad->Unbind();
	post_proc_shader->Unbind();
}
void Game::ShowImGui() {

	ImGui::Begin("Debug Menu");
	ImVec2 window_pos = ImGui::GetWindowPos();
	
	if (ImGui::CollapsingHeader("Game")) {
		ShowGameImGui();
	}
	if (ImGui::CollapsingHeader("Player")) {
		m_Player->ShowImGui();
	}
	if (ImGui::CollapsingHeader("World")) {
		m_World->ShowImGui();
	}
	if (ImGui::CollapsingHeader("Noise")) {
		static Ref<Texture> noise = nullptr;
		static uint32_t seed = 123467420;
		if (ImGui::Button("Generate")) noise = NoiseGenerator::GeneratePerlinNoise(50,50,2,{0,0}, seed);

		if (noise != nullptr) {
			ImGuiImage("Noise", noise);
		}
	}

	ImVec2 window_size = ImGui::GetWindowSize();
	window_size.x += window_pos.x;
	window_size.y += window_pos.y;
	ImGui::End();
	ImVec2 cursor_pos = ImGui::GetMousePos();
	
	m_MouseAvalible = (window_pos.x > cursor_pos.x || window_size.x < cursor_pos.x) || (window_pos.y > cursor_pos.y || window_size.y < cursor_pos.y);
	m_MouseAvalible &= !m_MouseCaptured;
}
void Game::ShowGameImGui() {
	if (ImGui::Button("Clear Debug Lines")) {
		ClearDebugLines();
	}
	if (ImGui::Button("Reload Shaders")) {
		LoadShaders();
	}
	static int s_NewWorldSeed = 643706557;
	ImGui::InputInt("Seed", &s_NewWorldSeed);
	if (ImGui::Button("New World")) {
		NewWorld(s_NewWorldSeed);
	}
}
void Game::ShowStatsOverlay() {
	glm::vec2 screen_min = UI::GetScreenMin();
	glm::vec2 screen_max = UI::GetScreenMax();

	std::string left_text;
	left_text += m_Player->StatsText();

	std::string right_text;
	right_text += "FPS " + std::to_string(m_FPS) + "\n";

	UI::MultilineText(left_text, glm::vec3(screen_max.x, screen_max.y, 0.5f), glm::vec2(0.5f), TextAlignment_Right);
	UI::MultilineText(right_text, glm::vec3(screen_min.x, screen_max.y, 0.5f), glm::vec2(0.5f), TextAlignment_Left);
}
void Game::ShutDown() {

}

void Game::LoadShaders() {

	m_Shaders[(size_t)ShaderType::PostProc] = CreateRef<Shader>("assets/shaders/PostProc.vert", "assets/shaders/PostProc.frag");
	m_Shaders[(size_t)ShaderType::World] = CreateRef<Shader>("assets/shaders/Main.vert", "assets/shaders/Main.frag");
	m_Shaders[(size_t)ShaderType::BlockPreview] = CreateRef<Shader>("assets/shaders/BlockPreview.vert", "assets/shaders/BlockPreview.frag");
	m_Shaders[(size_t)ShaderType::Selector] = CreateRef<Shader>("assets/shaders/Selector.vert", "assets/shaders/Selector.frag");
	m_Shaders[(size_t)ShaderType::SkyBox] = CreateRef<Shader>("assets/shaders/SkyBox.vert", "assets/shaders/SkyBox.frag");
	m_Shaders[(size_t)ShaderType::UIColored] = CreateRef<Shader>("assets/shaders/UI.vert", "assets/shaders/UIColored.frag");
	m_Shaders[(size_t)ShaderType::UITextured] = CreateRef<Shader>("assets/shaders/UI.vert", "assets/shaders/UITextured.frag");
	m_Shaders[(size_t)ShaderType::UIAtlas] = CreateRef<Shader>("assets/shaders/UI.vert", "assets/shaders/UIAtlas.frag");
	m_Shaders[(size_t)ShaderType::UIText] = CreateRef<Shader>("assets/shaders/UIText.vert", "assets/shaders/UIText.frag");

	m_DebugShader = CreateRef<Shader>("assets/shaders/DebugLine.vert", "assets/shaders/DebugLine.frag");
}
void Game::LoadTextures() {
	m_Textures[(size_t)TextureType::BlockAtlas] = CreateRef<Texture>("assets/textures/atlas.png");
	m_Textures[(size_t)TextureType::FontAtlas] = CreateRef<Texture>("assets/font/Font.png");
	m_Textures[(size_t)TextureType::CrossHair] = CreateRef<Texture>("assets/textures/cross_hair.png");
	m_Textures[(size_t)TextureType::Hotbar] = CreateRef<Texture>("assets/textures/hotbar.png");
	m_Textures[(size_t)TextureType::HotbarSelector] = CreateRef<Texture>("assets/textures/hotbar_selector.png");
	m_Textures[(size_t)TextureType::Inventory] = CreateRef<Texture>("assets/textures/inventory.png");
}
void Game::InitMenus() {
	// Pause menu
	Ref<Menu>& pause_menu = GetMenu(MenuType::Pause);
	pause_menu = CreateRef<Menu>("Pause Menu");
	pause_menu->AddButton({ "Return",  []() { Game::Get()->Resume(); } });
	pause_menu->AddButton({ "Options", []() { Game::SetActiveMenu(MenuType::Options); } });
	pause_menu->AddButton({ "Quit",    []() { Game::Get()->Quit(); } });
	pause_menu->End();

	// Options menu
	Ref<Menu>& options_menu = GetMenu(MenuType::Options);
	options_menu = CreateRef<Menu>("Options");
	options_menu->AddButton({ "The",     []() {} });
	options_menu->SameLine();
	options_menu->AddButton({ "Options", []() {} });
	options_menu->SameLine();
	options_menu->AddButton({ "Menu",    []() {} });
	options_menu->AddButton({ "Has",     []() {} });
	options_menu->SameLine();
	options_menu->AddButton({ "Not",     []() {} });
	options_menu->AddButton({ "Been",    []() {} });
	options_menu->SameLine();
	options_menu->AddButton({ "Created", []() {} });
	options_menu->SameLine();
	options_menu->AddButton({ "Yet...",  []() {} });
	options_menu->AddButton({ "Back",    []() { Game::SetActiveMenu(MenuType::Pause); } });
	options_menu->End();
}
void Game::NewWorld(uint32_t seed) {
	m_World = CreateRef<World>(seed);
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