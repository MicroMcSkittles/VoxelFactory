#pragma once
#include "Core/Core.h"
#include "Renderer/Texture.h"
#include <stdint.h>
#include <vector>

struct VertexAttribute {
	uint32_t type;
	int count;
	size_t GetSize() const;
};

struct VertexLayout {
	std::vector<VertexAttribute> attributes;
	size_t stride = 0;

	VertexLayout(const std::vector<VertexAttribute>& attributes);
};

class VertexBuffer {
public:
	VertexBuffer(const void* data, size_t size, const VertexLayout& vertex_layout);
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

class FrameBuffer {
public:
	FrameBuffer(int width, int height, int internal, int format);
	~FrameBuffer();

	void Resize(int width, int height);

	void Bind();
	void Unbind();

	Ref<Texture>& GetColorBuffer() { return m_ColorBuffer; }

private:
	uint32_t m_Handle;
	uint32_t m_RenderBufferHandle;
	Ref<Texture> m_ColorBuffer;
};