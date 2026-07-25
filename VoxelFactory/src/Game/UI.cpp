#include "Game/UI.h"
#include "Game/Game.h"
#include "Core/Utils.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <sstream>

Font::Font(const std::string& config_path) {
	std::ifstream file;
	file.open(config_path);
	ASSERT(file.is_open());

	ReadHeader(file);

	for (int i = 0; i < m_CharCount; i++) {
		ReadCharData(file);
	}

	file.close();
}
void Font::ReadHeader(std::ifstream& file) {

	std::string info_line, common_line, page_line, chars_line;
	std::getline(file, info_line);
	std::getline(file, common_line);
	std::getline(file, page_line);
	std::getline(file, chars_line);

	// Read size
	size_t size_loc = info_line.find("size=", 0) + 5;
	std::string size = info_line.substr(size_loc, info_line.find_first_of(' ', size_loc) - size_loc);
	m_Size = std::stoi(size);

	// Read line height
	size_t line_height_loc = common_line.find("lineHeight=", 0) + 11;
	std::string line_height = common_line.substr(line_height_loc, common_line.find_first_of(' ', line_height_loc) - line_height_loc);
	m_LineHeight = std::stof(line_height);
	m_LineHeight /= m_Size;

	// Read base
	size_t base_loc = common_line.find("base=", 0) + 5;
	std::string base = common_line.substr(base_loc, common_line.find_first_of(' ', base_loc) - base_loc);
	m_Base = std::stof(base);
	m_Base /= m_Size;

	// Read atlas size
	size_t scale_width_loc = common_line.find("scaleW=", 0) + 7;
	std::string scale_width = common_line.substr(scale_width_loc, common_line.find_first_of(' ', scale_width_loc) - scale_width_loc);
	m_AtlasSize.x = (float)std::stoi(scale_width);
	size_t scale_height_loc = common_line.find("scaleH=", 0) + 7;
	std::string scale_height = common_line.substr(scale_height_loc, common_line.find_first_of(' ', scale_height_loc) - scale_height_loc);
	m_AtlasSize.y = (float)std::stoi(scale_height);

	// Read char count
	size_t count_loc = chars_line.find("count=", 0) + 6;
	std::string count = chars_line.substr(count_loc, chars_line.find_first_of(' ', count_loc) - count_loc);
	m_CharCount = std::stoi(count);
}
void Font::ReadCharData(std::ifstream& file) {
	std::string data_line;
	std::getline(file, data_line);
	std::stringstream data(data_line);

	// Read char id
	std::string value;
	data >> value >> value;
	char id = (char)std::stoi(value.c_str() + 3);
	
	// Read atlas position
	glm::vec2 atlas_position = glm::vec2(0.0f);
	data >> value;
	atlas_position.x = std::stof(value.c_str() + 2);
	data >> value;
	atlas_position.y = std::stof(value.c_str() + 2);
	atlas_position /= m_AtlasSize;

	// Read atlas size
	glm::vec2 atlas_size = glm::vec2(0.0f);
	data >> value;
	atlas_size.x = std::stof(value.c_str() + 6);
	data >> value;
	atlas_size.y = std::stof(value.c_str() + 7);

	// Read offset
	glm::vec2 offset = glm::vec2(0.0f);
	data >> value;
	offset.x = std::stof(value.c_str() + 8);
	data >> value;
	offset.y = std::stof(value.c_str() + 8);
	offset /= m_Size;

	// Read x advance
	data >> value;
	float x_advance = std::stof(value.c_str() + 9);
	x_advance /= m_Size;

	glm::vec2 size = (atlas_size / (float)m_Size) * 0.5f;
	atlas_size /= m_AtlasSize;

	atlas_position.y = 1.0f - atlas_position.y - atlas_size.y;

	m_CharData.insert({ id, CharData{ offset, size, atlas_position, atlas_size, x_advance } });
}

void Button::Update() {
	GLFWwindow* window_handle = (GLFWwindow*)Game::GetWindow()->GetHandle();
	double mouse_x = 0.0, mouse_y = 0.0;
	glfwGetCursorPos(window_handle, &mouse_x, &mouse_y);

	glm::vec2 mouse_position = UI::GetWorldPosition(glm::vec2(mouse_x, mouse_y));

	hovered = (mouse_position.x >= position.x && mouse_position.x <= position.x + size.x * 2.0f &&
		       mouse_position.y >= position.y - size.y * 2.0f && mouse_position.y <= position.y);
	pressed = (hovered && glfwGetMouseButton(window_handle, GLFW_MOUSE_BUTTON_LEFT));

	if (pressed) callback();
}
void Button::Render() {
	glm::vec4 background_color = glm::vec4(0.55f, 0.55f, 0.55f, 1.0f);
	if (pressed) background_color = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);
	else if (hovered) background_color = glm::vec4(0.45f, 0.45f, 0.45f, 1.0f);
	UI::ColoredQuad(glm::vec3(position.x + size.x, position.y - size.y, position.z - 0.1f), size, background_color);
	UI::Text(text, glm::vec3(position.x + size.x, position.y - size.y * 0.3f, position.z), glm::vec2(0.4f), TextAlignment_Middle, glm::vec3(1.0f), glm::vec4(0.0f));
}

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
	m_FrameBuffer = CreateRef<FrameBuffer>(m_Width, m_Height, GL_RGBA8, GL_RGBA);

	// Create Quad
	m_Quad = CreateRef<VertexArray>();
	m_Quad->Bind();

	VertexLayout vertex_layout = { {
		{ GL_FLOAT, 2 },
		{ GL_FLOAT, 2 }
	} };
	Ref<VertexBuffer> vertex_buffer = CreateRef<VertexBuffer>(c_QuadVertices, 16 * sizeof(float), vertex_layout);
	m_Quad->GetVertexBuffer() = vertex_buffer;

	Ref<IndexBuffer> index_buffer = CreateRef<IndexBuffer>(c_QuadIndices, 6 * sizeof(uint32_t));
	m_Quad->GetIndexBuffer() = index_buffer;

	m_Quad->Unbind();

	m_ScreenMin = {
		-((float)m_Camera->view_box.width / (float)m_Camera->view_box.height) * m_Camera->view_box.scale,
		-m_Camera->view_box.scale
	};
	m_ScreenMax = {
		((float)m_Camera->view_box.width / (float)m_Camera->view_box.height) * m_Camera->view_box.scale,
		m_Camera->view_box.scale
	};
}

void UI::Resize(int width, int height) {
	s_Instance->m_Width = width;
	s_Instance->m_Height = height;
	s_Instance->m_Camera->view_box.width = width;
	s_Instance->m_Camera->view_box.height = height;
	s_Instance->m_Camera->UpdateProjection();

	s_Instance->m_FrameBuffer->Resize(width, height);

	s_Instance->m_ScreenMin = {
		-((float)s_Instance->m_Camera->view_box.width / (float)s_Instance->m_Camera->view_box.height) * s_Instance->m_Camera->view_box.scale,
		-s_Instance->m_Camera->view_box.scale
	};
	s_Instance->m_ScreenMax = {
		((float)s_Instance->m_Camera->view_box.width / (float)s_Instance->m_Camera->view_box.height) * s_Instance->m_Camera->view_box.scale,
		s_Instance->m_Camera->view_box.scale
	};
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

void UI::ColoredQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) {
	if (color.a == 0.0f) return;

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
void UI::TexturedQuad(const glm::vec3& position, float scale, const Ref<Texture>& texture) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);

	glm::vec3 size = {
		texture->GetAspectRatio() * scale,
		scale,
		1.0f
	};
	model = glm::scale(model, size);

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
void UI::AtlasQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture>& atlas, const glm::vec2& atlas_size, const glm::vec2& texture_coord) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::scale(model, glm::vec3(size, 1.0f));

	atlas->Bind();

	Ref<Shader>& ui_shader = Game::GetShader(ShaderType::UIAtlas);
	ui_shader->Bind();
	ui_shader->SetUniform("u_Model", model);
	ui_shader->SetUniform("u_ViewProjection", s_Instance->m_Camera->view_projection);

	uint32_t texture_id = (uint32_t)texture_coord.y * 16 + (uint32_t)texture_coord.x;
	ui_shader->SetUniform("u_AtlasWidth", (uint32_t)atlas_size.x);
	ui_shader->SetUniform("u_AtlasHeight", (uint32_t)atlas_size.y);
	ui_shader->SetUniform("u_TextureID", texture_id);
	ui_shader->SetUniform("u_Texture", atlas);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	ui_shader->Unbind();
	atlas->Unbind();
}
void UI::AtlasQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture>& atlas, const glm::vec2& atlas_size, uint32_t texture_id) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::scale(model, glm::vec3(size, 1.0f));

	atlas->Bind();

	Ref<Shader>& ui_shader = Game::GetShader(ShaderType::UIAtlas);
	ui_shader->Bind();
	ui_shader->SetUniform("u_Model", model);
	ui_shader->SetUniform("u_ViewProjection", s_Instance->m_Camera->view_projection);
	ui_shader->SetUniform("u_AtlasWidth", (uint32_t)atlas_size.x);
	ui_shader->SetUniform("u_AtlasHeight", (uint32_t)atlas_size.y);
	ui_shader->SetUniform("u_TextureID", texture_id);
	ui_shader->SetUniform("u_Texture", atlas);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	ui_shader->Unbind();
	atlas->Unbind();
}

void UI::Text(const std::string& text, const glm::vec3& position, const glm::vec2& size, TextAlignment alignment, const glm::vec3& forground, const glm::vec4& background) {
	
	// Setup instancing vars
	Ref<Font>& font = Game::GetFont();
	std::vector<glm::vec2> positions;
	std::vector<glm::vec2> sizes;
	std::vector<glm::vec2> atlas_positions;
	std::vector<glm::vec2> atlas_sizes;

	glm::vec2 cursor = glm::vec2(0.0f);
	cursor.y = font->GetLineHeight();

	// Create text instance data
	for (int i = 0; i < text.size(); i++) {
		const Font::CharData& data = font->GetCharData(text[i]);

		glm::vec2 position = {
			cursor.x + data.offset.x,
			cursor.y - data.offset.y - data.size.y
		};
		
		positions.push_back(position);
		sizes.push_back(data.size);
		atlas_positions.push_back(data.atlas_position);
		atlas_sizes.push_back(data.atlas_size);

		cursor.x += data.x_advance;
	}

	glm::vec3 text_offset = glm::vec3(0.5f * size.x, -font->GetLineHeight() * size.y, 0.0f);
	float text_padding = text_offset.x;
	glm::vec2 background_size = glm::vec2(cursor.x + text_padding, font->GetLineHeight()) * 0.5f * size;

	float alignment_offset = 0.0f;
	if (alignment == TextAlignment_Left) alignment_offset = 0.0f;
	else if (alignment == TextAlignment_Middle) alignment_offset = -background_size.x;
	else if (alignment == TextAlignment_Right) alignment_offset = -background_size.x * 2.0f;

	// Render background
	glm::vec3 background_position = position + glm::vec3(background_size, -0.05f);
	background_position.x += alignment_offset;
	background_position.y -= font->GetLineHeight() * size.y;
	ColoredQuad(background_position, background_size, background);

	// Render text
	Ref<Shader>& ui_shader = Game::GetShader(ShaderType::UIText);
	Ref<Texture> atlas = Game::GetTexture(TextureType::FontAtlas);
	ui_shader->Bind();
	atlas->Bind();

	// Calculate model matrix
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position + text_offset + glm::vec3(alignment_offset, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(size, 1.0f));

	// Set uniforms
	ui_shader->SetUniform("u_Model", model);
	ui_shader->SetUniform("u_ViewProjection", s_Instance->m_Camera->view_projection);
	ui_shader->SetUniform("u_FontAtlas", atlas);

	// Set color uniforms
	ui_shader->SetUniform("u_ForgroundColor", forground);
	ui_shader->SetUniform("u_BackgroundColor", background);

	// Set instancing uniforms
	ui_shader->SetUniform("u_Positions", positions);
	ui_shader->SetUniform("u_Sizes", sizes);
	ui_shader->SetUniform("u_AtlasPositions", atlas_positions);
	ui_shader->SetUniform("u_AtlasSizes", atlas_sizes);

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, text.size());

	atlas->Unbind();
	ui_shader->Unbind();
}

void UI::MultilineText(const std::string& text, const glm::vec3& position, const glm::vec2& size, TextAlignment alignment, const glm::vec3& forground, const glm::vec4& background) {
	std::vector<std::string> lines;
	size_t start = 0;
	size_t end = 0;
	while ((end = text.find_first_of("\n", start)) != std::string::npos) {
		lines.push_back(text.substr(start, end - start));
		start = end + 1;
	}

	float line_height = Game::GetFont()->GetLineHeight() * size.y;
	for (int i = 0; i < lines.size(); i++) {
		glm::vec3 line_position = position;
		line_position.y -= (float)i * line_height;
		Text(lines[i], line_position, size, alignment, forground, background);
	}
}

glm::vec2 UI::GetWorldPosition(const glm::vec2& position) {
	glm::vec2 world_position;
	world_position.x = (position.x * 2.0f) / (float)s_Instance->m_Width - 1.0f;
	world_position.y = (((float)s_Instance->m_Height - position.y) * 2.0f) / (float)s_Instance->m_Height - 1.0f;
	return glm::inverse(s_Instance->m_Camera->view_projection) * glm::vec4(world_position, 0.0f, 1.0f);
}
