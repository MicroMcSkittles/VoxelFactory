#pragma once
#include "Renderer/Buffers.h"
#include <iostream>

template<typename T>
class Mesh {
public:

	Mesh() {}
	Mesh(const std::vector<T>& vertices, const std::vector<uint32_t>& indices, const VertexLayout& layout) 
		: m_Vertices(vertices), m_Indices(indices), m_Layout(layout) { }
	~Mesh() { }

	void CreateVertexArray() {
		m_VertexArray = CreateRef<VertexArray>();
		m_VertexArray->Bind();

		Ref<VertexBuffer> vertex_buffer = CreateRef<VertexBuffer>(m_Vertices.data(), m_Vertices.size() * sizeof(T), m_Layout);
		m_VertexArray->GetVertexBuffer() = vertex_buffer;

		Ref<IndexBuffer> index_buffer = CreateRef<IndexBuffer>(m_Indices.data(), m_Indices.size() * sizeof(uint32_t));
		m_VertexArray->GetIndexBuffer() = index_buffer;

		m_VertexArray->Unbind();
	}

	Ref<VertexArray>& GetVertexArray() { return m_VertexArray; }
	VertexLayout& GetLayout() { return m_Layout; }
	std::vector<T>& GetVertices() { return m_Vertices; }
	std::vector<uint32_t>& GetIndices() { return m_Indices; }

private:
	Ref<VertexArray> m_VertexArray;
	VertexLayout m_Layout;
	std::vector<T> m_Vertices;
	std::vector<uint32_t> m_Indices;
};