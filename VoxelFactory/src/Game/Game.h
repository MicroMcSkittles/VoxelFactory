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

enum class ShaderType {
	PostProc,
	World,
	Selector,
	UIColored,
	UITextured,
	UIAtlas,
	Count
};
enum class TextureType {
	BlockAtlas,
	CrossHair,
	Count
};

class Game {
public:
	Game();
	~Game();

	static Game* Get() { return s_Instance; }
	static Ref<Shader>& GetShader(ShaderType type) { return s_Instance->m_Shaders[(size_t)type]; }
	static Ref<Texture>& GetTexture(TextureType type) { return s_Instance->m_Textures[(size_t)type]; }

	void Run();

	void OnResize(int width, int height);
	void OnMouseClick(int button, int action, int mods);
	void OnScroll(float delta);

	void StartUp();
	void Update(float delta_time);
	void ShowImGui();
	void ShutDown();

	void NewWorld(uint32_t seed);

	void ClearDebugLines();
	static void PushDebugLine(const DebugLine& line);
	void ShowDebugLines();

private:
	void ShowGameImGui();
	void LoadShaders();
	void LoadTextures();

	void OnLeftClick();
	void OnRightClick();

private:
	// Misc
	Ref<Window> m_Window;
	Ref<Player> m_Player;
	Ref<World> m_World;
	Ref<UI> m_UI;
	bool m_Running;

	// Rendering
	std::array<Ref<Shader>, (size_t)ShaderType::Count> m_Shaders;
	std::array<Ref<Texture>, (size_t)TextureType::Count> m_Textures;
	Ref<Camera> m_Camera;
	Ref<FrameBuffer> m_MainFrameBuffer;

	// Debuging
	Ref<Shader> m_DebugShader;
	Ref<VertexArray> m_DebugLineMesh;
	inline static std::vector<DebugLine> m_DebugLines;
	
	// Input
	bool m_Focused;
	bool m_MouseAvalible;
	glm::vec2 m_LastMousePos;

	inline static Game* s_Instance = nullptr;
};