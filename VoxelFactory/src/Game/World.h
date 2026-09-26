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
#include <unordered_map>
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>

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
	float size;

	glm::vec3 velocity;

	uint32_t block_id;
	glm::vec2 texture_offset;
	float life_span;

	float timer = 0.0f;
	bool on_ground = false;

	void Render(const Ref<Camera>& camera, float brightness);
	bool Update(float delta_time, const Ref<Player>& player);
};

struct PregeneratedBlock {
	glm::vec3 position;
	uint8_t id;
};
struct PregeneratedChunk {
	glm::vec2 position;
	std::vector<uint8_t> height_map;
	std::vector<PregeneratedBlock> blocks;
};

class WorldGenerator;
class ChunkMesher;
// TODO: stop using a vec3 for the chunk position, why did i write it like that in the first place...
class Chunk {
public:
	Chunk();
	Chunk(const glm::vec3& position, uint32_t seed);
	~Chunk();

	Block& At(const glm::vec3& position);

	void ClearLightLevels();
	uint8_t GetLightLevel(const glm::vec3& position);
	void SetLightLevel(const glm::vec3& position, uint8_t light_level);

	bool IsVoid(const glm::vec3& position);
	bool IsTransparent(const glm::vec3& position);
	bool IsValid(const glm::vec3& position); // Returns true if position is inbounds
	glm::vec3 ToWorld(const glm::vec3& position);

	const glm::vec3& GetPosition() { return m_Position; }
	static glm::vec3 GetBlockChunkPosition(const glm::vec3& position); // Returns the position of the chunk a block is in
	static glm::vec3 GetBlockLocalPosition(const glm::vec3& position); // Returns the local position of a block in a chunk
	static glm::vec3 GetBlockPosition(const glm::vec3& position); // Returns the position of the voxel a point is in
	static bool InBounds(const glm::vec3& position);

private:
	// TODO: store blocks in a better way
	// https://www.reddit.com/r/technicalminecraft/comments/gjioyz/how_does_minecraft_handle_chunks_from_a_memory/
	// is a good explenation of a way to do that
	std::vector<Block> m_Blocks;
	std::vector<uint8_t> m_LightLevels; // ranges 0-15
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

struct Structure {
	std::vector<glm::vec3> positions;
	std::vector<uint8_t> ids;
};

class World;
class WorldGenerator {
public:
	static void SetWorld(World* world) { s_World = world; }

	static void GenerateChunk(Chunk* chunk, uint32_t seed);
	static void GenerateColumn(const glm::vec2& position, int& column_height, Chunk* chunk, uint32_t seed);
	static void GenerateOreVains(Chunk* chunk, const std::vector<int>& height_map, int min, int max, int count, uint8_t ore_block_id, uint32_t seed);
	static void GenerateTrees(Chunk* chunk, const std::vector<int>& height_map, uint32_t seed);
	static void GenerateGrass(Chunk* chunk, const std::vector<int>& height_map, uint32_t seed);
	static void GenerateStructure(Chunk* chunk, const Structure& structure);

	static Structure CreateTree(Chunk* chunk, const glm::vec3& base_position, uint32_t seed);

private:
	inline static World* s_World;
};
class World {
public:
	World(uint32_t seed);
	~World();

	void ShowImGui();

	void Update(float delta_time, const Ref<Player>& player);
	void PhysicsUpdate(float delta_time, const Ref<Player>& player);
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
	void SpawnSurfaceParticals(const glm::vec3& position, const glm::vec3& normal, int count, uint32_t state);

private:
	CollisionResultData LineAABBIntersection(const glm::vec3& start_position, const glm::vec3& end_position, const AABB& aabb);
	CollisionResultData DynamicAABBIntersection(const AABB& aabb, const glm::vec3& velocity, const AABB& target);
	CollisionResultData RayAABBIntersection(const Ray& ray, const glm::vec3& aabb_min, const glm::vec3& aabb_max);
	
	void UpdateEntities(float delta_time, const Ref<Player>& player);
	void RenderEntities(const Ref<Camera>& camera);

	void UpdateParticals(float delta_time, const Ref<Player>& player);
	void RenderParticals(const Ref<Camera>& camera);

	void CalculateLightLevels(const glm::vec2& chunk_position);
	void PropagateLight(const glm::vec3& position);

	void CreateChunk(const glm::vec2& position);

	void InitSkyBox();

private:
	uint32_t m_Seed;

	glm::vec3 m_LoadedCenter; // The point in the middle of the currently loaded chunks
	int m_LoadedRadius;
	int m_LoadedWidth;
	int m_LoadedArea;

	// Chunk Data Vars
	std::unordered_map<glm::vec2, Chunk> m_Chunks;
	std::unordered_map<glm::vec2, Ref<Mesh<BlockVertex>>> m_ChunkMeshes;

	int m_EntityRenderDist;
	std::vector<ItemEntity> m_Entities;
	std::vector<Partical> m_Particals;

	// Sky box vars
	glm::vec3 m_SkyColor;
	glm::vec3 m_SkyHorizonColor;
	float m_Brightness;
	float m_FogDistance; // In terms of chunks

private:
	friend WorldGenerator;
};

class ChunkMesher {
public:
	ChunkMesher(Chunk* chunk, World* world);
	~ChunkMesher();

	Ref<Mesh<BlockVertex>> CreateMesh();

private:
	bool IsVoid(const glm::vec3& position);
	void MeshFace(const glm::vec3& position, const glm::vec3& face_dir, uint32_t id, uint8_t block_id, const BlockVertex* data);
	void MeshCrossMesh(const glm::vec3& position, uint8_t block_id);

private:
	Chunk* m_Chunk;
	World* m_World;

	std::vector<BlockVertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	uint32_t m_VertexOffset;
};