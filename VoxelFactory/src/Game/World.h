#pragma once
#include "Core/Core.h"
#include "Renderer/Buffers.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Renderer/Camera.h"
#include "Renderer/Mesh.h"
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

	static bool HasOrientation(uint8_t id);
	static int GetAxisCount(uint8_t id);
	static uint8_t CalculateOrientation(const glm::vec3& direction, uint8_t id);
	static glm::vec3 OrientVector(const glm::vec3& direction, int axis_count, uint8_t orientation);
};
class Chunk {
public:
	Chunk(const glm::vec3& position, uint32_t seed);
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
	// TODO: store blocks in a better way
	// https://www.reddit.com/r/technicalminecraft/comments/gjioyz/how_does_minecraft_handle_chunks_from_a_memory/
	// is a good explenation of a way to do that
	std::vector<Block> m_Blocks;
	glm::vec3 m_Position;

	friend ChunkMesher;

public:
	const static int ChunkLength = 16; // The width and length
	const static int ChunkArea = ChunkLength * ChunkLength; // The number of blocks in a horizontal slice of the chunk
	const static int ChunkHeight = 256;
	const static int ChunkDataSize = ChunkArea * ChunkHeight; // Total number of blocks
};
struct ChunkVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coord;
	float ambient_occlusion;
	uint32_t id;
};

struct AABB {
	glm::vec3 min;
	glm::vec3 max;
	glm::vec3 position;
	glm::vec3 size;

	void SetPosition(const glm::vec3& position) { this->position = position; CalculateMinMax(); }
	void SetSize(const glm::vec3& size) { this->size = size; CalculateMinMax(); }
	void CalculateMinMax();

	AABB();
	AABB(const glm::vec3& min, const glm::vec3& max, const glm::vec3& position, const glm::vec3& size);
	AABB(const glm::vec3& position, const glm::vec3& size);
};
struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
	glm::vec3 inv_direction;

	Ray(const glm::vec3& origin, const glm::vec3& direction) :
		origin(origin), direction(direction), inv_direction(1.0f / direction) { }
};
struct CollisionResultData {
	bool hit = false;
	float dist = 0.0f;
	glm::vec3 voxel_position = glm::vec3(0.0f);
	glm::vec3 normal = glm::vec3(0.0f);

	operator bool() { return hit; }
};

class World {
public:
	World(uint32_t seed);
	~World();

	void ShowImGui();

	void Update(const glm::vec3& position);
	void RenderSkyBox(const Ref<Camera>& camera);
	void RenderWorld(const Ref<Camera>& camera);

	void SetVoxel(const glm::vec3& position, uint8_t new_id);
	Chunk* GetChunk(const glm::vec3& position);
	void RebuildChunk(const glm::vec3& position);

	CollisionResultData CastRay(const Ray& ray);

	std::vector<glm::vec3> AABBIntersectedVoxels(const AABB& aabb);
	bool WillIntersect(const AABB& aabb, const glm::vec3& voxel);
	bool ResolveDynamicAABB(const AABB& aabb, glm::vec3& velocity, glm::vec3& normal);

	bool IsVoid(const glm::vec3& position);

private:
	CollisionResultData LineAABBIntersection(const glm::vec3& start_position, const glm::vec3& end_position, const AABB& aabb);
	CollisionResultData DynamicAABBIntersection(const AABB& aabb, const glm::vec3& velocity, const AABB& target);
	CollisionResultData RayAABBIntersection(const Ray& ray, const glm::vec3& aabb_min, const glm::vec3& aabb_max);
	
	void MoveLoadedCenter(const glm::vec2& delta);
	void CreateChunk(const glm::vec2& position);

	void InitSkyBox();

private:
	uint32_t m_Seed;

	glm::vec3 m_LoadedCenter; // The point in the middle of the currently loaded chunks
	int m_LoadedRadius;
	int m_LoadedWidth;
	int m_LoadedArea;

	std::vector<Chunk> m_Chunks;
	std::vector<Ref<Mesh<ChunkVertex>>> m_ChunkMeshes;

	// Sky box vars
	Ref<VertexArray> m_SkyBox;
	glm::vec3 m_SkyColor;
	glm::vec3 m_SkyHorizonColor;
	float m_Brightness;

private:
	const inline static float c_SkyBoxVertices[] = {
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f
	};
	const inline static uint32_t c_SkyBoxIndices[] = {
		0, 1, 3,
		1, 2, 3,
		4, 5, 7,
		5, 6, 7,
		0, 1, 4,
		1, 4, 5,
		2, 3, 7,
		2, 6, 7,
		0, 3, 4,
		3, 4, 7,
		1, 2, 5,
		2, 5, 6
	};
};

class ChunkMesher {
public:
	ChunkMesher(Chunk* chunk, World* world);
	~ChunkMesher();

	Ref<Mesh<ChunkVertex>> CreateMesh();

private:
	bool IsVoid(const glm::vec3& position);
	void MeshFace(const glm::vec3& position, const glm::vec3& face_dir, uint32_t id, uint8_t block_id, const ChunkVertex* data);

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
		{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
		{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } }
	};
	const inline static ChunkVertex c_BackVertices[] = {
		{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } }
	};
	const inline static ChunkVertex c_LeftVertices[] = {
		{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }
	};
	const inline static ChunkVertex c_RightVertices[] = {
		{ { -0.5f,  0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
		{ { -0.5f, -0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
		{ { -0.5f,  0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } }
	};
	const inline static ChunkVertex c_TopVertices[] = {
		{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
		{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } }
	};
	const inline static ChunkVertex c_BottomVertices[] = {
		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
		{ { -0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } }
	};

	const inline static uint32_t c_Indices[] = {
		0,  1,  2,
		3,  4,  5
	};
};