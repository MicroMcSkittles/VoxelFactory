#pragma once
#include <string>

class Texture {
public:
	Texture(const std::string& filename);
	Texture(const uint8_t* data, int width, int height, int internal, int format);
	~Texture();

	void Resize(int width, int height);

	void Bind();
	void Unbind();

	uint32_t GetHandle() { return m_Handle; }
	uint32_t GetSlot() { return m_Slot; }
	int GetWidth() { return m_Width; }
	int GetHeight() { return m_Height; }

private:
	uint32_t m_Handle;
	uint32_t m_Slot;
	int m_Internal;
	int m_Format;
	int m_Width;
	int m_Height;

private:
	const uint32_t c_MaxBound = 16;
	static uint32_t s_BoundCount;
};