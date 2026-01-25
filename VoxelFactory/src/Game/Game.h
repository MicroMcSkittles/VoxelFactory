#pragma once
#include "Core/Core.h"
#include "Core/Window.h"
#include "Renderer/Buffers.h"
#include "Renderer/Shader.h"
#include "Renderer/Camera.h"

class Game {
public:
	Game();
	~Game();

	void Run();

	void OnResize(int width, int height);

	void StartUp();
	void Update(double delta_time);
	void ShutDown();

private:
	Ref<Window> m_Window;

	Ref<Camera> m_Camera;
	Ref<Shader> m_MainShader;

	glm::mat4 m_Model;
	Ref<VertexArray> m_VAO;

	float m_Time;
	bool m_Running;
};