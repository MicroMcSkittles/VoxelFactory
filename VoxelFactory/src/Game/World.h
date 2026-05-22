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
	uint8_t id;

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

	const inline static uint8_t InvalidID = std::numeric_limits<uint8_t>::max();
	static Block Invalid;
};
class Chunk {
public:
	Chunk(const glm::vec3& position);
	~Chunk();

	Block& At(const glm::vec3& position);
	bool IsVoid(const glm::vec3& position);
	bool IsValid(const glm::vec3& position); // Returns true if position is inbounds
	glm::vec3 ToWorld(const glm::vec3& position);

	const glm::vec3& GetPosition() { return m_Position; }
	static glm::vec3 GetBlockChunkPosition(const glm::vec3& position); // Returns the position of the chunk a block is in
	static glm::vec3 GetBlockLocalPosition(const glm::vec3& position); // Returns the local position of a block in a chunk
	static glm::vec3 GetBlockPosition(const glm::vec3& position); // Returns the position of the voxel a point is in

private:
	std::vector<Block> m_Blocks;
	glm::vec3 m_Position;

	friend ChunkMesher;

public:
	const static int ChunkLength = 16; // The width and length
	const static int ChunkArea = ChunkLength * ChunkLength; // The number of blocks in a horizontal slice of the chunk
	const static int ChunkHeight = 256;
	const static int ChunkDataSize = ChunkArea * ChunkHeight; // Total number of blocks
};

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
	glm::vec3 inv_direction;

	Ray(const glm::vec3& origin, const glm::vec3& direction) :
		origin(origin), direction(direction), inv_direction(1.0f / direction) { }
};
struct RayResultData {
	bool hit = false;
	float dist = 0.0f;
	glm::vec3 voxel_position = glm::vec3(0.0f);
	glm::vec3 normal = glm::vec3(0.0f);

	operator bool() { return hit; }
};

class World {
public:
	World();
	~World();

	void Update(const glm::vec3& position);
	void Render(const Ref<Camera>& camera);

	Chunk* GetChunk(const glm::vec3& position);

	RayResultData CastRay(const Ray& ray);

private:
	RayResultData RayAABBIntersection(const Ray& ray, const glm::vec3& aabb_min, const glm::vec3& aabb_max);
	void LoadChunks(const glm::vec2& delta);

private:
	glm::vec3 m_LoadedCenter; // The point in the middle of the currently loaded chunks
	int m_LoadedRadius;
	int m_LoadedWidth;
	int m_LoadedArea;

	std::vector<Chunk> m_Chunks;
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
	void MeshFace(const glm::vec3& position, const glm::vec3& face_dir, uint32_t id, const ChunkVertex* data);

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