#pragma once
#include "Core/Core.h"
#include "Core/Window.h"
#include "Renderer/Buffers.h"
#include "Renderer/Shader.h"
#include "Renderer/Camera.h"
#include "Renderer/Texture.h"
#include "Game/World.h"
#include "Game/Player.h"
#include "Game/UI.h"
#include "Game/Menu.h"

#include <array>

struct DebugVertex {
	glm::vec3 position;
	glm::vec3 color;
};
struct DebugLine {
	DebugVertex vertex1;
	DebugVertex vertex2;

	DebugLine(const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& color):
		vertex1(point1, color), vertex2(point2, color) { }
};

enum class GameState {
	InGame,
	Paused,
	Menu
};

enum class ShaderType {
	PostProc,
	World,
	BlockPreview,
	Selector,
	SkyBox,
	UIColored,
	UITextured,
	UIAtlas,
	UIText,
	Count
};
enum class TextureType {
	BlockAtlas,
	FontAtlas,
	CrossHair,
	Hotbar,
	HotbarSelector,
	Inventory,
	Count
};
enum class MenuType {
	Pause,
	Options,
	Main,
	Count,
	None
};

class Game {
public:
	Game();
	~Game();

	// TODO: fix this
	static Game* Get() { return s_Instance; }
	static Ref<Window>& GetWindow() { return s_Instance->m_Window; }
	static Ref<Font>& GetFont() { return s_Instance->m_Font; }
	static Ref<Shader>& GetShader(ShaderType type) { return s_Instance->m_Shaders[(size_t)type]; }
	static Ref<Texture>& GetTexture(TextureType type) { return s_Instance->m_Textures[(size_t)type]; }
	static Ref<Menu>& GetMenu(MenuType type) { return s_Instance->m_Menus[(size_t)type]; }
	static void SetActiveMenu(MenuType type) { s_Instance->m_ActiveMenu = type; }
	static bool IsMouseCaptured() { return s_Instance->m_MouseCaptured; }

	static GameState& GetState() { return s_Instance->m_State; }
	void Pause();
	void Resume();
	void Quit();

	void Run();

	void OnResize(int width, int height);
	void OnMouseClick(int button, int action, int mods);
	void OnScroll(float delta);
	void OnKey(int key, int action, int mods);

	void StartUp();
	void Update(float delta_time);
	void Render();
	void ShowImGui();
	void ShutDown();

	void NewWorld(uint32_t seed);

	void CaptureMouse();
	void ReleaseMouse();

	void ClearDebugLines();
	static void PushDebugLine(const DebugLine& line);
	void ShowDebugLines();

private:
	void ShowGameImGui();
	void LoadShaders();
	void LoadTextures();
	void InitMenus();

	void OnLeftClick();
	void OnRightClick();

	void ShowStatsOverlay();

private:
	// Misc
	Ref<Window> m_Window;
	Ref<Player> m_Player;
	Ref<World> m_World;
	Ref<UI> m_UI;
	bool m_Running;
	GameState m_State;

	// Menus
	MenuType m_ActiveMenu;
	std::array<Ref<Menu>, (size_t)MenuType::Count> m_Menus;
	bool m_ShowStats;

	// Rendering
	Ref<Font> m_Font;
	std::array<Ref<Shader>, (size_t)ShaderType::Count> m_Shaders;
	std::array<Ref<Texture>, (size_t)TextureType::Count> m_Textures;
	Ref<Camera> m_Camera;
	Ref<FrameBuffer> m_MainFrameBuffer;

	// Debuging
	float m_FPS;
	Ref<Shader> m_DebugShader;
	Ref<VertexArray> m_DebugLineMesh;
	inline static std::vector<DebugLine> m_DebugLines;
	
	// Input
	bool m_MouseCaptured;
	bool m_MouseAvalible;
	glm::vec2 m_LastMousePos;

	inline static Game* s_Instance = nullptr;
};