#pragma once
#include <stdint.h>
#include <string>

class Window {
public:
	Window(uint32_t width, uint32_t height, const std::string& title);
	~Window();

	void Update();
	void MakeCurrent();
	bool ShouldClose();

	void* GetHandle() { return m_Handle; }
	uint32_t GetWidth() { return m_Width; }
	uint32_t GetHeight() { return m_Height; }

private:
	static void ErrorCallback(int error_code, const char* description);

private:
	void* m_Handle;
	uint32_t m_Width;
	uint32_t m_Height;

private:
	static bool s_GLFWInitialized;
	static size_t s_GLFWWindowCount;
};