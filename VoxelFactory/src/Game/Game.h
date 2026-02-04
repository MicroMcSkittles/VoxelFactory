#pragma once
#include "Core/Core.h"
#include "Core/Window.h"
#include "Renderer/Buffers.h"
#include "Renderer/Shader.h"
#include "Renderer/Camera.h"
#include "Renderer/Texture.h"
#include "Game/Chunk.h"

class Game {
public:
	Game();
	~Game();

	void Run();

	void OnResize(int width, int height);

	// Processes user input for camera
	void UpdateCamera(float delta_time);

	void StartUp();
	void Update(float delta_time);
	void ShutDown();

private:
	// Misc
	Ref<Window> m_Window;
	bool m_Running;

	// Rendering
	Ref<Camera> m_Camera;
	Ref<Shader> m_MainShader;

	glm::mat4 m_Model;
	Ref<Chunk> m_Chunk;
	Ref<VertexArray> m_VAO;
	Ref<Texture> m_Texture;

	// Input
	bool m_Focused;
	float m_CameraSpeed;
	float m_MouseSensitivity;
	glm::vec2 m_LastMousePos;
};