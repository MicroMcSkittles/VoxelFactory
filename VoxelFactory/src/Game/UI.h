#pragma once
#include "Core/Core.h"
#include "Renderer/Buffers.h"
#include "Renderer/Texture.h"
#include "Renderer/Camera.h"
#include <glm/glm.hpp>

class UI {
public:
	UI(int width, int height);
	~UI() { }

	static void Resize(int width, int height);

	static void StartFrame();
	static void EndFrame();

	static void ColoredQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec3& color);
	static void TexturedQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture>& texture);
	static void AtlasQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec2& texture_coord);
	static void AtlasQuad(const glm::vec3& position, const glm::vec2& size, uint32_t texture_id);

	static Ref<Texture>& GetFrame() { return s_Instance->m_FrameBuffer->GetColorBuffer(); }
	static Ref<VertexArray>& GetQuad() { return s_Instance->m_Quad; }

private:

	int m_Width;
	int m_Height;
	Ref<OrthographicCamera> m_Camera;
	Ref<FrameBuffer> m_FrameBuffer;
	Ref<VertexArray> m_Quad;

	inline static UI* s_Instance;

private:
	const inline static float c_QuadVertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 1.0f, 0.0f
	};
	const inline static uint32_t c_QuadIndices[] = {
		2, 1, 0,
		0, 3, 2
	};
};