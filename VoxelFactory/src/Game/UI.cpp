#include "Game/UI.h"
#include "Game/Game.h"
#include "Core/Utils.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

UI::UI(int width, int height)
	: m_Width(width), m_Height(height)
{
	ASSERT(s_Instance == nullptr);
	s_Instance = this;

	// Create Camera
	ViewBox view_box;
	view_box.width = m_Width;
	view_box.height = m_Height;
	view_box.near = 0.1f;
	view_box.far = 1000.0f;
	view_box.scale = 8.0f;
	m_Camera = CreateRef<OrthographicCamera>(view_box, glm::vec3(0.0f));

	// Create Frame Buffer
	s_Instance->m_FrameBuffer = CreateRef<FrameBuffer>(m_Width, m_Height, GL_RGBA8, GL_RGBA);

	// Create Quad
	s_Instance->m_Quad = CreateRef<VertexArray>();
	s_Instance->m_Quad->Bind();

	VertexLayout vertex_layout = { {
		{ GL_FLOAT, 2 },
		{ GL_FLOAT, 2 }
	} };
	Ref<VertexBuffer> vertex_buffer = CreateRef<VertexBuffer>(c_QuadVertices, 16 * sizeof(float), vertex_layout);
	s_Instance->m_Quad->GetVertexBuffer() = vertex_buffer;

	Ref<IndexBuffer> index_buffer = CreateRef<IndexBuffer>(c_QuadIndices, 6 * sizeof(uint32_t));
	s_Instance->m_Quad->GetIndexBuffer() = index_buffer;

	s_Instance->m_Quad->Unbind();
}

void UI::Resize(int width, int height) {
	s_Instance->m_Width = width;
	s_Instance->m_Height = height;
	s_Instance->m_Camera->view_box.width = width;
	s_Instance->m_Camera->view_box.height = height;
	s_Instance->m_Camera->UpdateProjection();

	s_Instance->m_FrameBuffer->Resize(width, height);
}

void UI::StartFrame() {
	s_Instance->m_FrameBuffer->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	s_Instance->m_Quad->Bind();
}
void UI::EndFrame() {
	s_Instance->m_Quad->Unbind();
	s_Instance->m_FrameBuffer->Unbind();
}

void UI::ColoredQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec3& color) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::scale(model, glm::vec3(size, 1.0f));

	Ref<Shader>& ui_shader = Game::GetShader(ShaderType::UIColored);
	ui_shader->Bind();
	ui_shader->SetUniform("u_Model", model);
	ui_shader->SetUniform("u_Color", color);
	ui_shader->SetUniform("u_ViewProjection", s_Instance->m_Camera->view_projection);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	ui_shader->Unbind();
}
void UI::TexturedQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture>& texture) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::scale(model, glm::vec3(size, 1.0f));

	texture->Bind();

	Ref<Shader>& ui_shader = Game::GetShader(ShaderType::UITextured);
	ui_shader->Bind();
	ui_shader->SetUniform("u_Model", model);
	ui_shader->SetUniform("u_Texture", texture);
	ui_shader->SetUniform("u_ViewProjection", s_Instance->m_Camera->view_projection);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	ui_shader->Unbind();
	texture->Unbind();
}
void UI::AtlasQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec2& texture_coord) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::scale(model, glm::vec3(size, 1.0f));

	Ref<Texture>& block_atlas = Game::GetTexture(TextureType::BlockAtlas);
	block_atlas->Bind();

	Ref<Shader>& ui_shader = Game::GetShader(ShaderType::UITextured);
	ui_shader->Bind();
	ui_shader->SetUniform("u_Model", model);
	ui_shader->SetUniform("u_ViewProjection", s_Instance->m_Camera->view_projection);

	uint32_t texture_id = (uint32_t)texture_coord.y * 16 + (uint32_t)texture_coord.x;
	ui_shader->SetUniform("u_TextureID", texture_id);
	ui_shader->SetUniform("u_Texture", block_atlas);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	ui_shader->Unbind();
	block_atlas->Unbind();
}
void UI::AtlasQuad(const glm::vec3& position, const glm::vec2& size, uint32_t texture_id) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::scale(model, glm::vec3(size, 1.0f));

	Ref<Texture>& block_atlas = Game::GetTexture(TextureType::BlockAtlas);
	block_atlas->Bind();

	Ref<Shader>& ui_shader = Game::GetShader(ShaderType::UIAtlas);
	ui_shader->Bind();
	ui_shader->SetUniform("u_Model", model);
	ui_shader->SetUniform("u_ViewProjection", s_Instance->m_Camera->view_projection);
	ui_shader->SetUniform("u_TextureID", texture_id);
	ui_shader->SetUniform("u_Texture", block_atlas);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	ui_shader->Unbind();
	block_atlas->Unbind();
}
