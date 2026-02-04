#pragma once
#include "Core/Core.h"
#include "Renderer/Buffers.h"
#include <vector>
#include <glm/glm.hpp>

struct Block {
	uint16_t id;
	const inline static uint16_t InvalidID = std::numeric_limits<uint16_t>::max();
	static Block Invalid;
};

class ChunkMesher;

class Chunk {
public:
	Chunk();
	~Chunk();

	Block& At(const glm::vec3& position);
	bool IsVoid(const glm::vec3& position);
	bool IsValid(const glm::vec3& position);

	const glm::vec3& GetPosition() { return m_Position; }

private:
	std::vector<Block> m_Blocks;
	glm::vec3 m_Position;

	friend ChunkMesher;

public:
	const static int ChunkWidth = 16;
	const static int ChunkLength = 16;
	const static int ChunkHeight = 16;
	const static int ChunkDataSize = ChunkWidth * ChunkHeight * ChunkLength;
};

struct ChunkVertex {
	glm::vec3 position;
	glm::vec2 tex_coord;
	uint32_t id;
};
class ChunkMesher {
public:
	ChunkMesher(const Ref<Chunk>& chunk);
	~ChunkMesher();

	Ref<VertexArray> Mesh();

private:
	void MeshFace(const glm::vec3& position, const glm::vec3& face, const ChunkVertex* data);

private:
	Ref<Chunk> m_Chunk;
	
	std::vector<ChunkVertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	uint32_t m_VertexOffset;

private:
	const inline static size_t c_FaceVertexCount = 6;
	const inline static size_t c_FaceIndexCount = 6;

	const inline static ChunkVertex c_FrontVertices[] = {
		{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f } },
		{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f } },
		{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } }
	};
	const inline static ChunkVertex c_BackVertices[] = {
		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } }
	};
	const inline static ChunkVertex c_LeftVertices[] = {
		{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f } }
	}; 
	const inline static ChunkVertex c_RightVertices[] = {
		{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f } },
		{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
		{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f } }
	};
	const inline static ChunkVertex c_TopVertices[] = {
		{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f } },
		{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } }
	};
	const inline static ChunkVertex c_BottomVertices[] = {
		{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
		{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f } }
	};

	const inline static uint32_t c_Indices[] = {
		0,  1,  2,
		3,  4,  5
	};
};