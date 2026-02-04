#include "Renderer/Buffers.h"

#include <glad/glad.h>

VertexBuffer::VertexBuffer(const void* data, size_t size) {
	glGenBuffers(1, &m_Handle);
	glBindBuffer(GL_ARRAY_BUFFER, m_Handle);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}
VertexBuffer::~VertexBuffer() {
	glDeleteBuffers(1, &m_Handle);
}

void VertexBuffer::Bind() {
	glBindBuffer(GL_ARRAY_BUFFER, m_Handle);
}
void VertexBuffer::Unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

IndexBuffer::IndexBuffer(const uint32_t* data, size_t size) {
	glGenBuffers(1, &m_Handle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Handle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	m_Count = size / sizeof(uint32_t);
}
IndexBuffer::~IndexBuffer() {
	glDeleteBuffers(1, &m_Handle);
}

void IndexBuffer::Bind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Handle);
}
void IndexBuffer::Unbind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

VertexArray::VertexArray() {
	glCreateVertexArrays(1, &m_Handle);
}
VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &m_Handle);
}

void VertexArray::Bind() {
	glBindVertexArray(m_Handle);
}
void VertexArray::Unbind() {
	glBindVertexArray(0);
}