#pragma once
#include "Core/Core.h"
#include "Renderer/Texture.h"
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
	Shader(const std::string& vertex_path, const std::string& fragment_path);
	~Shader();

	void SetUniform(const std::string& name, const glm::vec3& value);
	void SetUniform(const std::string& name, const glm::mat4& value);
	void SetUniform(const std::string& name, const Ref<Texture>& value);

	void Bind();
	void Unbind();

	uint32_t GetHandle() { return m_Handle; }

private:
	uint32_t m_Handle;

#ifdef DEBUG
	bool m_Bound;
#endif
};