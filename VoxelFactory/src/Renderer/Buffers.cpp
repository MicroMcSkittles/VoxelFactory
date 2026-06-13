#include "Renderer/Buffers.h"

#include <glad/glad.h>

size_t VertexAttribute::GetSize() const {

	size_t type_size = 0;
	switch (type) {
	case GL_FLOAT: type_size = sizeof(float); break;
	case GL_UNSIGNED_INT: type_size = sizeof(unsigned int); break;
	default: type_size = 0;
	}

	return type_size * count;
}
VertexLayout::VertexLayout(const std::vector<VertexAttribute>& attributes): attributes(attributes) {
	stride = 0;
	for (VertexAttribute attrib : attributes) {
		stride += attrib.GetSize();
	}
}

VertexBuffer::VertexBuffer(const void* data, size_t size, const VertexLayout& vertex_layout) {
	glGenBuffers(1, &m_Handle);
	glBindBuffer(GL_ARRAY_BUFFER, m_Handle);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

	size_t offset = 0;
	
	for (uint32_t i = 0; i < vertex_layout.attributes.size(); i++) {
		const VertexAttribute& attrib = vertex_layout.attributes[i];
		if (attrib.type == GL_UNSIGNED_INT)
			glVertexAttribIPointer(i, attrib.count, attrib.type, vertex_layout.stride, (void*)offset);
		else 
			glVertexAttribPointer(i, attrib.count, attrib.type, GL_FALSE, vertex_layout.stride, (void*)offset);
		glEnableVertexAttribArray(i);
		offset += attrib.GetSize();
	}

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

