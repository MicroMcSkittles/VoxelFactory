#pragma once
#include "Core/Core.h"
#include "Renderer/Buffers.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Renderer/Camera.h"
#include "Renderer/Mesh.h"
#include "Game/Voxel.h"
#include "Game/Inventory.h"
#include <vector>
#include <glm/glm.hpp>
#include <thread>
#include <mutex>

class Player;
struct ItemEntity {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 size;
	glm::vec3 velocity;

	Item item;
	bool player_dropped;

	glm::vec3 offset = glm::vec3(0.0f);
	float timer = 0.0f;
	bool on_ground = false;

	void Render(const Ref<Camera>& camera, float brightness);
	bool Update(float delta_time, const Ref<Player>& player);
};

struct Partical {
	glm::vec3 position;
	glm::vec3 velocity;
	uint32_t block_id;
	glm::vec2 texture_offset;
	float life_span;

	float timer = 0.0f;
	bool on_ground = false;

	void Render(const Ref<Camera>& camera, float brightness);
	bool Update(float delta_time, const Ref<Player>& player);
};

class WorldGenerator;
class ChunkMesher;
class Chunk {
public:
	Chunk(const glm::vec3& position, uint32_t seed);
	~Chunk();

	Block& At(const glm::vec3& position);
	bool IsVoid(const glm::vec3& position);
	bool IsTransparent(const glm::vec3& position);
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
	friend WorldGenerator;

public:
	const static int ChunkLength = 16; // The width and length
	const static int ChunkArea = ChunkLength * ChunkLength; // The number of blocks in a horizontal slice of the chunk
	const static int ChunkHeight = 256;
	const static int ChunkDataSize = ChunkArea * ChunkHeight; // Total number of blocks
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

class WorldGenerator {
public:
	static void GenerateChunk(Chunk* chunk, uint32_t seed);
	static void GenerateColumn(const glm::vec2& position, Chunk* chunk, uint32_t seed);
};
class World {
public:
	World(uint32_t seed);
	~World();

	void ShowImGui();

	void Update(float delta_time, const Ref<Player>& player);
	void RenderSkyBox(const Ref<Camera>& camera);
	void RenderWorld(const Ref<Camera>& camera);

	Block& GetVoxel(const glm::vec3& position);
	void SetVoxel(const glm::vec3& position, uint8_t new_id);
	void BreakVoxel(const glm::vec3& position);
	Chunk* GetChunk(const glm::vec3& position);
	void RebuildChunk(const glm::vec3& position);

	CollisionResultData CastRay(const Ray& ray);

	std::vector<glm::vec3> AABBIntersectedVoxels(const AABB& aabb);
	bool WillIntersect(const AABB& aabb, const glm::vec3& voxel);
	bool ResolveDynamicAABB(const AABB& aabb, glm::vec3& velocity, glm::vec3& normal);

	bool IsVoid(const glm::vec3& position);
	bool IsTransparent(const glm::vec3& position);

	float& GetBrightness() { return m_Brightness; }

	void CreateItem(const Item& item, const glm::vec3& position, const glm::vec3& velocity, bool player_dropped);

private:
	CollisionResultData LineAABBIntersection(const glm::vec3& start_position, const glm::vec3& end_position, const AABB& aabb);
	CollisionResultData DynamicAABBIntersection(const AABB& aabb, const glm::vec3& velocity, const AABB& target);
	CollisionResultData RayAABBIntersection(const Ray& ray, const glm::vec3& aabb_min, const glm::vec3& aabb_max);
	
	void UpdateEntities(float delta_time, const Ref<Player>& player);
	void RenderEntities(const Ref<Camera>& camera);

	void UpdateParticals(float delta_time, const Ref<Player>& player);
	void RenderParticals(const Ref<Camera>& camera);

	void MoveLoadedCenter(const glm::vec2& delta);
	void BuildChunks();
	void LoadChunk();
	void CreateChunk(const glm::vec2& position);
	void CheckChunkLoaderThread();

	void InitSkyBox();

private:
	uint32_t m_Seed;

	glm::vec3 m_LoadedCenter; // The point in the middle of the currently loaded chunks
	int m_LoadedRadius;
	int m_LoadedWidth;
	int m_LoadedArea;

	std::vector<Chunk> m_Chunks;
	std::vector<Ref<Mesh<BlockVertex>>> m_ChunkMeshes;

	int m_EntityRenderDist;
	std::vector<ItemEntity> m_Entities;
	std::vector<Partical> m_Particals;

	// Sky box vars
	Ref<VertexArray> m_SkyBox;
	glm::vec3 m_SkyColor;
	glm::vec3 m_SkyHorizonColor;
	float m_Brightness;

	// Chunk loading multithreading vars
	inline static bool s_ThreadsFinished = false;
	std::thread m_ChunkLoaderThread;
	inline static std::mutex s_ChunkLoaderMutex;
	inline static std::vector<glm::vec2> s_ChunksToLoad;
	inline static std::vector<glm::vec2> s_ChunksToRebuild;
	inline static std::vector<glm::vec2> s_ChunksRebuilt;

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

	Ref<Mesh<BlockVertex>> CreateMesh();

private:
	bool IsVoid(const glm::vec3& position);
	void MeshFace(const glm::vec3& position, const glm::vec3& face_dir, uint32_t id, uint8_t block_id, const BlockVertex* data);
	void MeshFlower(const glm::vec3& position, uint8_t block_id);

private:
	Chunk* m_Chunk;
	World* m_World;

	std::vector<BlockVertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	uint32_t m_VertexOffset;
};