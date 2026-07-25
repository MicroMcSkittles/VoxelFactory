#pragma once
#include "Core/Core.h"
#include "Core/Utils.h"
#include "Renderer/Buffers.h"
#include "Renderer/Texture.h"
#include "Renderer/Camera.h"
#include "Renderer/Mesh.h"
#include <glm/glm.hpp>
#include <map>

class Font {
public:
	struct CharData {
		glm::vec2 offset;
		glm::vec2 size;
		glm::vec2 atlas_position;
		glm::vec2 atlas_size;
		float x_advance;
	};

public:
	Font(const std::string& config_path);
	~Font() { }

	const CharData& GetCharData(char c) { return m_CharData[c]; }
	int GetSize() { return m_Size; }
	float GetLineHeight() { return m_LineHeight; }
	float GetBase() { return m_Base; }

private:
	void ReadHeader(std::ifstream& file);
	void ReadCharData(std::ifstream& file);

private:
	int m_CharCount;
	int m_Size;
	float m_LineHeight;
	float m_Base;
	glm::vec2 m_AtlasSize;
	std::map<char, CharData> m_CharData;
};

enum TextAlignment {
	TextAlignment_Left   = 0,
	TextAlignment_Middle = 1,
	TextAlignment_Right  = 2
};

struct Button {
	std::string text;

	glm::vec3 position;
	glm::vec2 size;

	bool pressed;
	bool hovered;

	std::function<void()> callback;

	Button(const std::string& text, const std::function<void()>& callback) :
		text(text), position({ 0,0,0 }), size({ 0,0 }), callback(callback), pressed(false), hovered(false) {
	}
	Button(const std::string& text, const glm::vec3& position, const glm::vec2& size, const std::function<void()>& callback) :
		text(text), position(position), size(size), callback(callback), pressed(false), hovered(false) { }
	
	void Update();
	void Render();
};

class UI {
public:
	UI(int width, int height);
	~UI() { }

	static void Resize(int width, int height);

	static void StartFrame();
	static void EndFrame();

	static void ColoredQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
	static void TexturedQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture>& texture);
	static void TexturedQuad(const glm::vec3& position, float scale, const Ref<Texture>& texture);
	static void AtlasQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture>& atlas, const glm::vec2& atlas_size, const glm::vec2& texture_coord);
	static void AtlasQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture>& atlas, const glm::vec2& atlas_size, uint32_t texture_id);

	static void Text(const std::string& text, const glm::vec3& position, const glm::vec2& size, TextAlignment alignment = TextAlignment_Middle, const glm::vec3& forground = glm::vec3(1.0f), const glm::vec4& background = glm::vec4(0.69f,0.69f,0.69f,0.75f));
	static void MultilineText(const std::string& text, const glm::vec3& position, const glm::vec2& size, TextAlignment alignment = TextAlignment_Middle, const glm::vec3& forground = glm::vec3(1.0f), const glm::vec4& background = glm::vec4(0.69f, 0.69f, 0.69f, 0.75f));

	static Ref<Texture>& GetFrame() { return s_Instance->m_FrameBuffer->GetColorBuffer(); }
	static Ref<VertexArray>& GetQuad() { return s_Instance->m_Quad; }

	static glm::vec2 GetWorldPosition(const glm::vec2& position);
	static glm::vec2 GetScreenMin() { return s_Instance->m_ScreenMin; }
	static glm::vec2 GetScreenMax() { return s_Instance->m_ScreenMax; }

private:

	int m_Width;
	int m_Height;
	glm::vec2 m_ScreenMin;
	glm::vec2 m_ScreenMax;

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