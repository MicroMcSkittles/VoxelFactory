#pragma once
#include <stdint.h>

class VertexBuffer {
public:
	VertexBuffer(const void* data, size_t size);
	~VertexBuffer();

	void Bind();
	void Unbind();

	uint32_t GetHandle() { return m_Handle; }

private:
	uint32_t m_Handle;
};

class IndexBuffer {
public:
	IndexBuffer(const uint32_t* data, size_t size);
	~IndexBuffer();

	void Bind();
	void Unbind();

	uint32_t GetHandle() { return m_Handle; }

private:
	uint32_t m_Handle;
};

class VertexArray {
public:
	VertexArray();
	~VertexArray();

	void Bind();
	void Unbind();

private:
	uint32_t m_Handle;
};