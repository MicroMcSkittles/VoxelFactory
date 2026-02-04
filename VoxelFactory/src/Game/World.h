#pragma once
#include "Core/Core.h"
#include "Renderer/Buffers.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Renderer/Camera.h"
#include <vector>
#include <glm/glm.hpp>

class ChunkMesher;

struct Block {
	uint16_t id;

	struct TextureIDs {
		uint32_t front;
		uint32_t back;

		uint32_t left;
		uint32_t right;

		uint32_t top;
		uint32_t bottom;

		TextureIDs(uint32_t id): front(id), back(id), left(id), right(id), top(id), bottom(id) { }
		TextureIDs(uint32_t sides, uint32_t top, uint32_t bottom): front(sides), back(sides), left(sides), right(sides), top(top), bottom(bottom) {}
		TextureIDs(uint32_t front, uint32_t back, uint32_t left, uint32_t right, uint32_t top, uint32_t bottom): front(front), back(back), left(left), right(right), top(top), bottom(bottom) { }
	};
	static std::vector<TextureIDs> BlockTextureIDs;

	const inline static uint16_t InvalidID = std::numeric_limits<uint16_t>::max();
	static Block Invalid;
};
class Chunk {
public:
	Chunk(const glm::vec3& position);
	~Chunk();

	Block& At(const glm::vec3& position);
	bool IsVoid(const glm::vec3& position);
	bool IsValid(const glm::vec3& position);
	glm::vec3 ToWorld(const glm::vec3& position);

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

class World {
public:
	World();
	~World();

	Chunk* GetChunk(const glm::vec3& position);

	void Render(const Ref<Camera>& camera);

private:
	glm::vec3 m_LoadedCenter; // The point in the middle of the currently loaded chunks
	int m_LoadedRadius;  // The radius around m_LoadedCenter where chunks are loaded

	std::vector<Chunk> m_LoadedChunks;
	std::vector<Ref<VertexArray>> m_ChunkMeshes;

	Ref<Shader> m_MainShader;
	Ref<Texture> m_Atlas;
};

struct ChunkVertex {
	glm::vec3 position;
	glm::vec2 tex_coord;
	uint32_t id;
};
class ChunkMesher {
public:
	ChunkMesher(Chunk* chunk, World* world);
	~ChunkMesher();

	Ref<VertexArray> Mesh();

private:
	void MeshFace(const glm::vec3& position, const glm::vec3& face, uint32_t id, const ChunkVertex* data);

private:
	Chunk* m_Chunk;
	World* m_World;
	
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
		{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } }
	};
	const inline static ChunkVertex c_LeftVertices[] = {
		{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } }
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
		{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
		{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f } }
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