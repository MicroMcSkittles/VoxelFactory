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

	void ShowImGui();
	void Update(float delta_time, glm::vec2& last_mouse_pos, const Ref<Window>& window);
	void Render();
	void RenderUI(Ref<Shader>& ui_shader);

	void OnLeftClick();
	void OnRightClick();
private:

	// Update velocity based on input
	void Input(float delta_time, const Ref<Window>& window);
	// Update camera rotation based on mouse movement
	void CameraInput(glm::vec2& last_mouse_pos, const Ref<Window>& window);

	void InitUI();

	void InitSelector();
	void RenderSelector();

private:
	
	// General
	Ref<World> m_World;
	Ref<Camera> m_Camera;
	
	Ref<Texture> m_CrossHair;

	glm::vec3 m_Position;
	glm::vec3 m_CameraOffset;
	
	// Physics
	glm::vec3 m_ColliderOffset;
	AABB m_Collider;
	glm::vec3 m_Velocity;
	float m_GravitationalConstant;
	float m_Drag;

	float m_GroundCheckDist;
	bool m_OnGround;

	// Selector
	Ref<Shader> m_SelectorShader;
	Ref<VertexArray> m_SelectorMesh;
	glm::vec3 m_SelectorPosition;
	bool m_ShowSelector;

	// Controls
	bool m_Flight;
	float m_MouseSensitivity;
	float m_WalkSpeed;
	float m_SprintMultiplier;
	float m_Speed;
	float m_JumpForce;
	float m_Reach;

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