#pragma once
#include "Core/Core.h"
#include "Core/Window.h"
#include "Renderer/Buffers.h"
#include "Renderer/Shader.h"
#include "Renderer/Camera.h"
#include "Renderer/Texture.h"
#include "Game/World.h"
#include "Game/Player.h"

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

class Game {
public:
	Game();
	~Game();

	void Run();

	void OnResize(int width, int height);
	void OnMouseClick(int button, int action, int mods);

	void StartUp();
	void Update(float delta_time);
	void ShowImGui();
	void ShutDown();

	void ClearDebugLines();
	static void PushDebugLine(const DebugLine& line);
	void ShowDebugLines();

private:
	void OnLeftClick();
	void OnRightClick();

private:
	// Misc
	Ref<Window> m_Window;
	Ref<Player> m_Player;
	bool m_Running;

	// Rendering
	Ref<Camera> m_Camera;
	Ref<World> m_World;

	Ref<Shader> m_DebugShader;
	Ref<VertexArray> m_DebugLineMesh;
	inline static std::vector<DebugLine> m_DebugLines;

	// Input
	bool m_Focused;
	bool m_MouseAvalible;
	glm::vec2 m_LastMousePos;

};