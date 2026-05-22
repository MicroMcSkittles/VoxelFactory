#pragma once
#include "Core/Core.h"
#include "Core/Window.h"
#include "Game/World.h"
#include "Renderer/Camera.h"
#include "Renderer/Buffers.h"
#include "Renderer/Shader.h"

class Player {
public:

	Player(const Ref<Camera>& camera, const Ref<World>& world);

	void Update(float delta_time, glm::vec2& last_mouse_pos, const Ref<Window>& window);

private:
	void HandleLeftClick();
	void HandleRightClick();
	void InitSelector();
	void RenderSelector();

private:

	glm::vec3 m_Position;

	Ref<World> m_World;
	Ref<Camera> m_Camera;

	Ref<Shader> m_SelectorShader;
	Ref<VertexArray> m_SelectorMesh;
	glm::vec3 m_SelectorPosition;
	bool m_ShowSelector;

	// Controlls
	bool m_MouseLClickLast;
	bool m_MouseRClickLast;
	float m_MouseSensitivity;
	float m_Speed;

private:
	const inline static float c_SelectorVertices[] = {
		-0.52f, -0.52f, -0.52f, 0.0f, 0.0f, 0.0f,
		-0.52f,  0.52f, -0.52f, 0.0f, 0.0f, 0.0f,

		 0.52f, -0.52f, -0.52f, 0.0f, 0.0f, 0.0f,
		 0.52f,  0.52f, -0.52f, 0.0f, 0.0f, 0.0f,

		-0.52f, -0.52f,  0.52f, 0.0f, 0.0f, 0.0f,
		-0.52f,  0.52f,  0.52f, 0.0f, 0.0f, 0.0f,

		 0.52f, -0.52f,  0.52f, 0.0f, 0.0f, 0.0f,
		 0.52f,  0.52f,  0.52f, 0.0f, 0.0f, 0.0f
	};
	const inline static uint32_t c_SelectorIndices[] = {
		0, 1, 1, 3, 3, 2, 2, 0,
		0, 4, 1, 5, 2, 6, 3, 7,
		4, 5, 5, 7, 7, 6, 6, 4
	};
};