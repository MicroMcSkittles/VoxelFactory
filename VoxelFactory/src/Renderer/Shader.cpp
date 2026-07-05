#include "Renderer/Shader.h"
#include "Core/Utils.h"
#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

std::string ReadSource(const std::string& filename) {
	std::ifstream file;
	file.open(filename);
	ASSERT_MSG(file.is_open(), "Failed to open file \"{}\"", filename);

	std::stringstream content_buffer;
	content_buffer << file.rdbuf();
	file.close();

	return content_buffer.str();
}

Shader::Shader(const std::string& vertex_path, const std::string& fragment_path) {

	// Load source
	std::string vertex_source = ReadSource(vertex_path);
	std::string fragment_source = ReadSource(fragment_path);
	const char* vertex_c_cource = vertex_source.c_str();
	const char* fragment_c_cource = fragment_source.c_str();

	int success;
	char info[512];

	// Load vertex shader
	uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_c_cource, nullptr);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex_shader, 512, nullptr, info);
		ASSERT_MSG(false, "A OpenGL error occured: {}", info);
	}

	// Load fragment shader
	uint32_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_c_cource, nullptr);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment_shader, 512, nullptr, info);
		ASSERT_MSG(false, "A OpenGL error occured: {}", info);
	}

	m_Handle = glCreateProgram();
	glAttachShader(m_Handle, vertex_shader);
	glAttachShader(m_Handle, fragment_shader);
	glLinkProgram(m_Handle);
	glDeleteProgram(vertex_shader);
	glDeleteProgram(fragment_shader);

	glGetShaderiv(m_Handle, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(m_Handle, 512, nullptr, info);
		ASSERT_MSG(false, "A OpenGL error occured: {}", info);
	}

#ifdef DEBUG
	m_Bound = false;
#endif
}
Shader::~Shader() {
	glDeleteProgram(m_Handle);
}

void Shader::SetUniform(const std::string& name, uint32_t value) {
	ASSERT_MSG(m_Bound, "A OpenGL error occured: failed to set uniform unsigned int, shader not bound");
	uint32_t location = glGetUniformLocation(m_Handle, name.c_str());
	glUniform1ui(location, value);
}
void Shader::SetUniform(const std::string& name, float value) {
	ASSERT_MSG(m_Bound, "A OpenGL error occured: failed to set uniform float, shader not bound");
	uint32_t location = glGetUniformLocation(m_Handle, name.c_str());
	glUniform1f(location, value);
}
void Shader::SetUniform(const std::string& name, const glm::vec3& value) {
	ASSERT_MSG(m_Bound, "A OpenGL error occured: failed to set uniform vec3, shader not bound");
	uint32_t location = glGetUniformLocation(m_Handle, name.c_str());
	glUniform3f(location, value.x, value.y, value.z);
}
void Shader::SetUniform(const std::string& name, const glm::vec4& value) {
	ASSERT_MSG(m_Bound, "A OpenGL error occured: failed to set uniform vec3, shader not bound");
	uint32_t location = glGetUniformLocation(m_Handle, name.c_str());
	glUniform4f(location, value.x, value.y, value.z, value.w);
}
void Shader::SetUniform(const std::string& name, const glm::mat4& value) {
	ASSERT_MSG(m_Bound, "A OpenGL error occured: failed to set uniform mat4, shader not bound");
	uint32_t location = glGetUniformLocation(m_Handle, name.c_str());
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::SetUniform(const std::string& name, const Ref<Texture>& value) {
	ASSERT_MSG(m_Bound, "A OpenGL error occured: failed to set uniform texture, shader not bound");
	uint32_t location = glGetUniformLocation(m_Handle, name.c_str());
	glUniform1i(location, value->GetSlot());
}

void Shader::SetUniform(const std::string& name, const std::vector<glm::vec2>& value_list) {
	//ASSERT_MSG(m_Bound, "A OpenGL error occured: failed to set uniform vec2 list, shader not bound");
	//uint32_t location = glGetUniformLocation(m_Handle, name.c_str());
	//glUniform2fv(location, value_list.size(), &value_list[0].x);
	for (int i = 0; i < value_list.size(); i++) {
		SetUniformElement(name, i, value_list[i]);
	}
}
void Shader::SetUniformElement(const std::string& name, int index, const glm::vec2& value) {
	ASSERT_MSG(m_Bound, "A OpenGL error occured: failed to set uniform vec2 element, shader not bound");
	uint32_t location = glGetUniformLocation(m_Handle, (name + "[" + std::to_string(index) + "]").c_str());
	glUniform2f(location, value.x, value.y);
}

void Shader::Bind() {
	glUseProgram(m_Handle);
#ifdef DEBUG
	m_Bound = true;
#endif
}
void Shader::Unbind() {
	glUseProgram(0);
#ifdef DEBUG
	m_Bound = false;
#endif
}
