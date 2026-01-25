#include "Renderer/Texture.h"
#include "Core/Utils.h"

#include <stb/stb_image.h>
#include <glad/glad.h>

uint32_t Texture::s_BoundCount = 0;

Texture::Texture(const std::string& filename) {
	glGenTextures(1, &m_Handle);
	glBindTexture(GL_TEXTURE_2D, m_Handle);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int nrChannels;
	uint8_t* data = stbi_load(filename.c_str(), &m_Width, &m_Height, &nrChannels, STBI_rgb_alpha);
	ASSERT_MSG(data, "A OpenGL error occured: Failed to create texture \"{}\"", filename);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	m_Slot = std::numeric_limits<uint32_t>::max();
}
Texture::~Texture() {
	glDeleteTextures(1, &m_Handle);
}

void Texture::Bind() {
	ASSERT_MSG(s_BoundCount != c_MaxBound, "A OpenGL error occured: To many bound textures");
	glActiveTexture(GL_TEXTURE0 + s_BoundCount);
	glBindTexture(GL_TEXTURE_2D, m_Handle);
	s_BoundCount += 1;
}
void Texture::Unbind() {
	s_BoundCount -= 1;
	m_Slot = std::numeric_limits<uint32_t>::max();
}