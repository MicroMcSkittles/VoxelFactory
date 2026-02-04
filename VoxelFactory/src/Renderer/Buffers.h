#pragma once
#include "Core/Core.h"
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
	uint32_t GetCount() { return m_Count; }

private:
	uint32_t m_Handle;
	uint32_t m_Count;
};

class VertexArray {
public:
	VertexArray();
	~VertexArray();

	void Bind();
	void Unbind();

	Ref<VertexBuffer>& GetVertexBuffer() { return m_VertexBuffer; }
	Ref<IndexBuffer>& GetIndexBuffer() { return m_IndexBuffer; }

private:
	uint32_t m_Handle;
	Ref<VertexBuffer> m_VertexBuffer;
	Ref<IndexBuffer> m_IndexBuffer;
};