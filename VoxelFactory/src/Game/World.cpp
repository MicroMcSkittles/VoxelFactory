#include "Game/World.h"
#include "Core/Utils.h"
#include "Core/ImGuiUtils.h"
#include "Game/Noise.h"
#include "Game/Game.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <chrono>
#include <unordered_set>

#include <imgui.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtx/euler_angles.hpp>

AABB::AABB() : min(0.0f), max(0.0f), position(0.0f), size(0.0f) { }
AABB::AABB(const glm::vec3& min, const glm::vec3& max, const glm::vec3& position, const glm::vec3& size)
	: min(min), max(max), position(position), size(size) { }
AABB::AABB(const glm::vec3& position, const glm::vec3& size)
	: position(position), size(size) 
{
	CalculateMinMax();
}

void AABB::CalculateMinMax() {
	min = {
		position.x - size.x * 0.5f,
		position.y - size.y * 0.5f,
		position.z - size.z * 0.5f
	};
	max = {
		position.x + size.x * 0.5f,
		position.y + size.y * 0.5f,
		position.z + size.z * 0.5f
	};
}

void ItemEntity::Render(const Ref<Camera>& camera, float brightness) {
	Ref<Shader> shader = Game::GetShader(ShaderType::BlockPreview);
	Ref<Texture> atlas = Game::GetTexture(TextureType::BlockAtlas);
	shader->Bind();
	atlas->Bind();

	// Calculate model matrix
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position + offset);
	model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, size);

	// Set uniforms
	shader->SetUniform("u_Model", model);
	shader->SetUniform("u_ViewProjection", camera->view_projection);
	shader->SetUniform("u_Brightness", brightness);
	shader->SetUniform("u_Atlas", atlas);

	const TextureIDs& texture_ids = Block::GetTextureIDs(item.id);
	shader->SetUniform("u_TextureIDs", texture_ids.List());

	// Figure out how many items to layer
	float count = (float)item.count / (float)Item::StackSize;
	if (item.count > 1 && count < 0.33f) count = 2;
	else if (count >= 0.33f) count = 3;

	MeshType type = MeshType::Block;
	if (Block::HasProperty(item.id, BlockProperty_CrossMesh)) type = MeshType::CrossMesh;
	Ref<VertexArray>& block_mesh = Game::GetMesh(type);

	block_mesh->Bind();
	glDisable(GL_CULL_FACE);
	
	glDrawElements(GL_TRIANGLES, block_mesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	
	if (count > 1) {
		model = glm::translate(model, glm::vec3(0.15f));
		shader->SetUniform("u_Model", model);
		glDrawElements(GL_TRIANGLES, block_mesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}
	if (count > 2) {
		model = glm::translate(model, glm::vec3(-0.2f, 0.2f, -0.275f));
		shader->SetUniform("u_Model", model);
		glDrawElements(GL_TRIANGLES, block_mesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}

	glEnable(GL_CULL_FACE);
	block_mesh->Unbind();

	atlas->Unbind();
	shader->Unbind();
}
bool ItemEntity::Update(float delta_time, const Ref<Player>& player) {

	// Gravity
	if (!on_ground) velocity.y += -0.388f * delta_time;
	// Drag
	velocity -= velocity * 0.901f * delta_time * (on_ground ? 9.0f : 1.0f);

	glm::vec3 player_direction = player->GetCameraPosition() - position;
	float player_dist = glm::length(player_direction);
	if ((timer >= 2.0f || !player_dropped) && player->HasItemSpace(item)) {
		// Move item towards player
		if (player_dist <= 3.0f) {
			velocity += glm::normalize(player_direction) * delta_time;
		}
		// Put item in players inventory
		if (player_dist <= 1.0f) {
			player->PushInventoryItems(item);
			if (item.count == 0) return true;
		}
	}

	// Apply animation
	offset.y = sin(timer * 0.65f) * 0.125f + (on_ground ? size.y : 0.0f);
	rotation.y += delta_time * 0.25f;
	if (rotation.y >= PI2) rotation.y -= PI2;

	timer += delta_time;
	return false;
}

void Partical::Render(const Ref<Camera>& camera, float brightness) {
	Ref<Shader> shader = Game::GetShader(ShaderType::Partical);
	Ref<Texture> atlas = Game::GetTexture(TextureType::BlockAtlas);
	shader->Bind();
	atlas->Bind();

	// Calculate billboard angles
	glm::vec3 camera_direction = position - camera->position;
	float theta = glm::atan(camera_direction.x, camera_direction.z);
	float phi = glm::atan(-camera_direction.y, glm::length(camera_direction));

	// Calculate model matrix
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::rotate(model, theta, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, phi, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(size * 0.1f));

	// Set uniforms
	shader->SetUniform("u_Model", model);
	shader->SetUniform("u_ViewProjection", camera->view_projection);
	shader->SetUniform("u_Brightness", brightness);
	shader->SetUniform("u_Atlas", atlas);
	shader->SetUniform("u_TextureID", Block::GetTextureIDs(block_id).front);
	shader->SetUniform("u_Offset", texture_offset);

	Ref<VertexArray>& quad_mesh = Game::GetMesh(MeshType::Quad);
	quad_mesh->Bind();

	glDisable(GL_CULL_FACE);
	glDrawElements(GL_TRIANGLES, quad_mesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	glEnable(GL_CULL_FACE);

	quad_mesh->Unbind();

	atlas->Unbind();
	shader->Unbind();
}
bool Partical::Update(float delta_time, const Ref<Player>& player) {
	// Gravity
	if (!on_ground) velocity.y += -0.388f * delta_time;
	// Drag
	velocity -= velocity * 0.901f * delta_time * (on_ground ? 14.0f : 1.0f);

	// Delete partical if old enough
	if (timer >= life_span) return true;
	timer += delta_time;
	return false;
}

Chunk::Chunk() {
	m_Position = glm::vec3(0.0f);
}
Chunk::Chunk(const glm::vec3& position, uint32_t seed) {
	m_Position = position;

	m_Blocks.resize(ChunkDataSize, Block{ 0 });
	m_LightLevels.resize(ChunkDataSize, MaxLightLevel);

	WorldGenerator::GenerateChunk(this, seed);
}
Chunk::~Chunk() { }

Block& Chunk::At(const glm::vec3& position) {
	if (position.x < 0 || position.x >= ChunkLength) return Block::Invalid;
	if (position.y < 0 || position.y >= ChunkHeight) return Block::Invalid;
	if (position.z < 0 || position.z >= ChunkLength) return Block::Invalid;
	size_t index = position.x + position.z * ChunkLength + position.y * ChunkArea;
	if (index >= ChunkDataSize) return Block::Invalid;
	return m_Blocks[index];
}
void Chunk::ClearLightLevels() {
	m_LightLevels.clear();
	m_LightLevels.resize(ChunkDataSize, 0);
}
uint8_t Chunk::GetLightLevel(const glm::vec3& position) {
	if (position.x < 0 || position.x >= ChunkLength) return 0;
	if (position.y < 0 || position.y >= ChunkHeight) return 0;
	if (position.z < 0 || position.z >= ChunkLength) return 0;
	size_t index = position.x + position.z * ChunkLength + position.y * ChunkArea;
	if (index >= ChunkDataSize) return 0;
	return m_LightLevels[index] & 0xF;
}
void Chunk::SetLightLevel(const glm::vec3& position, uint8_t light_level) {
	if (position.x < 0 || position.x >= ChunkLength) return;
	if (position.y < 0 || position.y >= ChunkHeight) return;
	if (position.z < 0 || position.z >= ChunkLength) return;
	size_t index = position.x + position.z * ChunkLength + position.y * ChunkArea;
	if (index >= ChunkDataSize) return;
	m_LightLevels[index] = light_level & 0xF;
}
bool Chunk::IsVoid(const glm::vec3& position) {
	Block& block = At(position);
	return block.id == Block::InvalidID || block.id == BlockID_Air;
}
bool Chunk::IsTransparent(const glm::vec3& position) {
	Block& block = At(position);
	return block.id == Block::InvalidID || block.id == BlockID_Air || Block::HasProperty(block.id, BlockProperty_Transparent);
}
bool Chunk::IsValid(const glm::vec3& position) {
	if (position.x < 0 || position.x >= ChunkLength) return false;
	if (position.y < 0 || position.y >= ChunkHeight) return false;
	if (position.z < 0 || position.z >= ChunkLength) return false;
	return true;
}
bool Chunk::InBounds(const glm::vec3& position) {
	if (position.x < 0 || position.x >= ChunkLength) return false;
	if (position.y < 0 || position.y >= ChunkHeight) return false;
	if (position.z < 0 || position.z >= ChunkLength) return false;
	return true;
}
glm::vec3 Chunk::ToWorld(const glm::vec3& position) {
	return {
		position.x + m_Position.x * ChunkLength,
		position.y,
		position.z + m_Position.z * ChunkLength
	};
}
glm::vec3 Chunk::GetBlockChunkPosition(const glm::vec3& position) {
	return { floor((position.x) / Chunk::ChunkLength), 0, floor((position.z) / Chunk::ChunkLength) };
}
glm::vec3 Chunk::GetBlockLocalPosition(const glm::vec3& position) {
	glm::vec3 chunk_pos = Chunk::GetBlockChunkPosition(position);
	return position - chunk_pos * (float)(Chunk::ChunkLength);
}
glm::vec3 Chunk::GetBlockPosition(const glm::vec3& position) {
	return { floor(position.x), floor(position.y), floor(position.z) };
}

World::World(uint32_t seed) : m_Seed(seed), m_LoadedCenter({ 0,0,0 }), m_EntityRenderDist(3) {
#ifdef DEBUG
	m_LoadedRadius = 5;
#else
	m_LoadedRadius = 15;
#endif

	m_LoadedWidth = m_LoadedRadius * 2 + 1;
	m_LoadedArea = m_LoadedWidth * m_LoadedWidth;

	WorldGenerator::SetWorld(this);
	m_Chunks.reserve(m_LoadedArea);
	m_ChunkMeshes.reserve(m_LoadedArea);

	// Generate chunk data
	for (int z = -m_LoadedRadius; z <= m_LoadedRadius; z++) {
		for (int x = -m_LoadedRadius; x <= m_LoadedRadius; x++) {
			m_Chunks.insert({ {x,z}, Chunk{ { m_LoadedCenter.x + x, 0, m_LoadedCenter.z + z}, m_Seed} });
		}
	}
	for (int z = -m_LoadedRadius; z <= m_LoadedRadius; z++) {
		for (int x = -m_LoadedRadius; x <= m_LoadedRadius; x++) {
			CalculateLightLevels({ m_LoadedCenter.x + x, m_LoadedCenter.z + z });
		}
	}
	// Generate chunk meshes
	for (auto& [key, chunk] : m_Chunks) {
		m_ChunkMeshes.insert({ key, ChunkMesher(&chunk, this).CreateMesh() });
		m_ChunkMeshes[key]->CreateVertexArray();
	}

	InitSkyBox();
}
World::~World() { }

void World::InitSkyBox() {
	m_SkyColor = glm::vec3(0.470f, 0.655f, 1.0f);
	m_SkyHorizonColor = glm::vec3(0.753f, 0.847f, 1.0f);
	m_Brightness = 1.0f;
	m_FogDistance = 14.0f;
}

Block& World::GetVoxel(const glm::vec3& position) {
	// Get chunk
	glm::vec3 chunk_position = Chunk::GetBlockChunkPosition(position);
	Chunk* chunk = GetChunk(chunk_position);
	if (chunk == nullptr) return Block::Invalid;

	glm::vec3 local_voxel_position = Chunk::GetBlockLocalPosition(position);
	return chunk->At(local_voxel_position);
}

void World::SetVoxel(const glm::vec3& position, uint8_t new_id) {
	glm::vec3 local_position = Chunk::GetBlockLocalPosition(position);
	glm::vec3 chunk_position = Chunk::GetBlockChunkPosition(position);
	Chunk* chunk = GetChunk(chunk_position);
	if (chunk == nullptr) return;
	chunk->At(local_position).id = new_id;

	// Rebuild effected chunks
	RebuildChunk(chunk_position);

	// Adjacent corner
	if (local_position.x == 0 && local_position.z == 0)
		RebuildChunk({ chunk_position.x - 1, 0, chunk_position.z - 1 });
	else if (local_position.x == Chunk::ChunkLength - 1 && local_position.z == Chunk::ChunkLength - 1)
		RebuildChunk({ chunk_position.x + 1, 0, chunk_position.z + 1 });

	// Adjacent side x
	if (local_position.x == 0)
		RebuildChunk({ chunk_position.x - 1, 0, chunk_position.z });
	else if (local_position.x == Chunk::ChunkLength - 1)
		RebuildChunk({ chunk_position.x + 1, 0, chunk_position.z });
	
	// Adjacent side z
	if (local_position.z == 0)
		RebuildChunk({ chunk_position.x, 0, chunk_position.z - 1 });
	else if (local_position.z == Chunk::ChunkLength - 1)
		RebuildChunk({ chunk_position.x, 0, chunk_position.z + 1 });
}
void World::BreakVoxel(const glm::vec3& position) {
	Block block = GetVoxel(position);
	block.id = block.id & ~Block::OrientationMask;

	// TODO: spawn particals on the surface of the block
	if (Block::HasProperty(block.id, BlockProperty_Unbreakable)) return;

	// Generate random item position and offset
	uint32_t state = ((int)position.x << (int)position.y) ^ (int)position.z;
	glm::vec3 item_offset = NoiseGenerator::RandomFloat3Range(state, 0.25f, 0.75f);
	glm::vec3 item_velocity = (item_offset - 0.5f) * 0.2f;
	item_velocity.y = abs(item_velocity.y) + 0.05f;

	CreateItem({ block.id, 1 }, position + item_offset, item_velocity, false);

	// Spawn particals
	for (int i = 0; i < 16; i++) {
		glm::vec3 offset = NoiseGenerator::RandomFloat3(state);
		Partical partical;
		partical.position = position + offset;
		partical.size = 1.0f;
		partical.velocity = (offset - 0.5f) * 0.15f;
		partical.life_span = 0.2f + NoiseGenerator::RandomFloatRange(state, -0.1f, 0.75f);
		partical.block_id = block.id;
		partical.texture_offset = NoiseGenerator::RandomFloat2Range(state, 0.0f, 0.75f);
		m_Particals.push_back(partical);
	}
	
	SetVoxel(position, 0);
}

Chunk* World::GetChunk(const glm::vec3& position) {
	if (!m_Chunks.count({ position.x, position.z })) return nullptr;
	return &m_Chunks[{ position.x, position.z }];
}
void World::RebuildChunk(const glm::vec3& position) {
	Chunk* chunk = GetChunk(position);
	if (chunk == nullptr) return;

	glm::vec2 key = { position.x, position.z };
	CalculateLightLevels(key);

	m_ChunkMeshes[key] = ChunkMesher(chunk, this).CreateMesh();
	m_ChunkMeshes[key]->CreateVertexArray();
}

CollisionResultData World::LineAABBIntersection(const glm::vec3& start_position, const glm::vec3& end_position, const AABB& aabb) {

	// Find the distances from start position to the greater planes
	glm::vec3 direction = end_position - start_position;
	glm::vec3 near_dist = (aabb.position - (aabb.size * 0.5f) - start_position) / direction;
	glm::vec3 far_dist = (aabb.position + (aabb.size * 0.5f) - start_position) / direction;

	// Order values properly
	if (far_dist.x < near_dist.x) std::swap(far_dist.x, near_dist.x);
	if (far_dist.y < near_dist.y) std::swap(far_dist.y, near_dist.y);
	if (far_dist.z < near_dist.z) std::swap(far_dist.z, near_dist.z);
	
	if (near_dist.x > far_dist.y) return { };
	if (near_dist.y > far_dist.x) return { };
	if (near_dist.z > far_dist.x) return { };

	if (near_dist.x > far_dist.z) return { };
	if (near_dist.y > far_dist.z) return { };
	if (near_dist.z > far_dist.y) return { };

	// Find distance to the entry point and exit point
	float near_hit_dist = std::max({ near_dist.x, near_dist.y, near_dist.z });
	float far_hit_dist = std::min({ far_dist.x, far_dist.y, far_dist.z });

	// Discard if didn't intersect on the line
	if (far_hit_dist < 0) return { };
	if (near_hit_dist > 1) return { };

	// Find normal
	glm::vec3 normal = glm::vec3(0.0f);
	if (near_dist.x > near_dist.y && near_dist.x > near_dist.z) {
		if (direction.x < 0) normal = { 1.0f, 0.0f, 0.0f };
		else normal = { -1.0f, 0.0f, 0.0f };
	}
	else if (near_dist.y > near_dist.z) {
		if (direction.y < 0) normal = { 0.0f, 1.0f, 0.0f };
		else normal = { 0.0f, -1.0f, 0.0f };
	}
	else {
		if (direction.z < 0) normal = { 0.0f, 0.0f, 1.0f };
		else normal = { 0.0f, 0.0f, -1.0f };
	}

	return { true, near_hit_dist, glm::vec3(0.0f), normal };
}
CollisionResultData World::DynamicAABBIntersection(const AABB& aabb, const glm::vec3& velocity, const AABB& target) {
	AABB expanded_target = { target.position, target.size + aabb.size };
	return LineAABBIntersection(aabb.position, aabb.position + velocity, expanded_target);
}
CollisionResultData World::RayAABBIntersection(const Ray& ray, const glm::vec3& aabb_min, const glm::vec3& aabb_max) {
	
	float min_dist = 0.0f;
	float max_dist = std::numeric_limits<float>::max();

	for (int i = 0; i < 3; i++) {
		float dist_t1 = (aabb_min[i] - ray.origin[i]) * ray.inv_direction[i];
		float dist_t2 = (aabb_max[i] - ray.origin[i]) * ray.inv_direction[i];

		min_dist = std::max(min_dist, std::min(dist_t1, dist_t2));
		max_dist = std::min(max_dist, std::max(dist_t1, dist_t2));
	}

	if (max_dist >= min_dist) {
		return { true, min_dist };
	}
	return { };
}

void World::CreateItem(const Item& item, const glm::vec3& position, const glm::vec3& velocity, bool player_dropped) {
	m_Entities.push_back({ position, glm::vec3(0.0f), glm::vec3(0.3f), velocity, item, player_dropped });
}

void World::SpawnSurfaceParticals(const glm::vec3& position, const glm::vec3& normal, int count, uint32_t state) {
	uint32_t rand_state = state << ((((int)position.x >> state) << (int)position.y) ^ (int)position.z);
	uint8_t block_id = GetVoxel(position).id;

	for (int i = 0; i < count; i++) {
		Partical partical;
		partical.size = 0.5f;
		partical.block_id = block_id;
		partical.life_span = 0.2f + NoiseGenerator::RandomFloatRange(rand_state, -0.1f, 0.35f);
		partical.texture_offset = NoiseGenerator::RandomFloat2Range(rand_state, 0.0f, 0.75f);

		glm::vec2 offset = NoiseGenerator::RandomFloat2Range(rand_state, -0.45f, 0.45f);
		glm::vec2 velocity = NoiseGenerator::RandomFloat2Range(rand_state, -0.025f, 0.025f);
		partical.position = (position + 0.5f) + (normal * 0.56f);
		if (normal.x != 0.0f) {
			partical.position.z += offset.x;
			partical.position.y += offset.y;
			
			partical.velocity.x = (normal.x * -0.05f) + velocity.x;
			partical.velocity.y = 0.025f;
			partical.velocity.z = velocity.y;
		}
		else if (normal.y != 0.0f) {
			partical.position.x += offset.x;
			partical.position.z += offset.y;

			partical.velocity.x = velocity.x;
			partical.velocity.y = 0.025f;
			partical.velocity.z = velocity.y;
		}
		else if (normal.z != 0.0f) {
			partical.position.x += offset.x;
			partical.position.y += offset.y;

			partical.velocity.x = velocity.x;
			partical.velocity.y = 0.025f;
			partical.velocity.z = (normal.z * -0.05f) + velocity.y;
		}

		m_Particals.push_back(partical);
	}
}

CollisionResultData World::CastRay(const Ray& ray) {

	// Find current chunk position
	float initial_dist = 0.0f;
	glm::vec3 origin = ray.origin;
	glm::vec3 chunk_position = Chunk::GetBlockChunkPosition(ray.origin);
	Chunk* current_chunk = GetChunk(chunk_position);

	// If ray is outside of the voxel grid than find the point the ray enters the grid
	if (current_chunk == nullptr) {
		// Find bounding box for all loaded chunks
		glm::vec3 aabb_min = {
			-m_LoadedRadius * Chunk::ChunkLength,
			0,
			-m_LoadedRadius * Chunk::ChunkLength
		};
		glm::vec3 aabb_max = {
			(m_LoadedRadius + 1) * Chunk::ChunkLength,
			Chunk::ChunkHeight,
			(m_LoadedRadius + 1) * Chunk::ChunkLength
		};
		CollisionResultData aabb_result = RayAABBIntersection(ray, aabb_min, aabb_max);
		if (!aabb_result) return { false };

		// Find entry point
		glm::vec3 entry_point = ray.origin + (ray.direction * aabb_result.dist);
		initial_dist = aabb_result.dist;
		origin = entry_point;
		chunk_position = Chunk::GetBlockChunkPosition(entry_point);
		current_chunk = GetChunk(chunk_position);
	}

	// Find origin positions
	glm::vec3 local_position = Chunk::GetBlockLocalPosition(origin);
	glm::vec3 voxel_position = Chunk::GetBlockLocalPosition(Chunk::GetBlockPosition(origin));

	// Find step values based on sign of direction, 0 if direction == 0
	glm::vec3 step = {
		(ray.direction.x > 0) - (ray.direction.x < 0),
		(ray.direction.y > 0) - (ray.direction.y < 0),
		(ray.direction.z > 0) - (ray.direction.z < 0)
	};

	// Find which planes the ray will pass through
	glm::vec3 plane = voxel_position + (step * 0.5f) + glm::vec3(0.5f);
	
	// Find initial distances from planes
	glm::vec3 max_dist = plane - local_position;
	max_dist.x *= ray.inv_direction.x;
	max_dist.y *= ray.inv_direction.y;
	max_dist.z *= ray.inv_direction.z;

	// Find distances between planes along the ray
	glm::vec3 delta_dist = { fabsf(ray.inv_direction.x), fabsf(ray.inv_direction.y), fabsf(ray.inv_direction.z) };
	
	// Traversal loop
	CollisionResultData result;
	while (current_chunk != nullptr && current_chunk->IsValid(voxel_position)) {
		// Exit loop if a voxel is hit
		if (!current_chunk->IsVoid(voxel_position)) {
			result.hit = true;
			result.voxel_position = current_chunk->ToWorld(voxel_position);
			result.dist += initial_dist;
			break;
		}

		// Step along the x-axis
		if (max_dist.x < max_dist.y && max_dist.x < max_dist.z) {
			result.dist = max_dist.x;
			result.normal = { -step.x,0,0 };
			voxel_position.x += step.x;
			max_dist.x += delta_dist.x;

			// Move to new chunk if boundery is crossed
			if (voxel_position.x < 0 || voxel_position.x >= Chunk::ChunkLength) {
				voxel_position.x = (step.x > 0) ? 0 : Chunk::ChunkLength - 1;
				chunk_position.x += step.x;
				current_chunk = GetChunk(chunk_position);
			}
		}
		// Step along the z-axis
		else if (max_dist.z < max_dist.y || std::isnan(max_dist.y)) {
			result.dist = max_dist.z;
			result.normal = { 0,0,-step.z };
			voxel_position.z += step.z;
			max_dist.z += delta_dist.z;

			// Move to new chunk if boundery is crossed
			if (voxel_position.z < 0 || voxel_position.z >= Chunk::ChunkLength) {
				voxel_position.z = (step.z > 0) ? 0 : Chunk::ChunkLength - 1;
				chunk_position.z += step.z;
				current_chunk = GetChunk(chunk_position);
			}
		}
		// Step along the y-axis
		else {
			result.dist = max_dist.y;
			result.normal = { 0,-step.y,0 };
			voxel_position.y += step.y;
			max_dist.y += delta_dist.y;
		}
	}

	return result;
}

std::vector<glm::vec3> World::AABBIntersectedVoxels(const AABB& aabb)
{
	// Get the min and max of effected area
	glm::vec3 min_voxel = Chunk::GetBlockPosition(aabb.min);
	glm::vec3 max_voxel = Chunk::GetBlockPosition(aabb.max);

	std::vector<glm::vec3> intersected;

	// Loop through each effected voxel
	glm::vec3 voxel_position = glm::vec3(0.0f);
	for (voxel_position.y = min_voxel.y; voxel_position.y <= max_voxel.y; voxel_position.y++) {
		for (voxel_position.z = min_voxel.z; voxel_position.z <= max_voxel.z; voxel_position.z++) {
			for (voxel_position.x = min_voxel.x; voxel_position.x <= max_voxel.x; voxel_position.x++) {

				// Find chunk the voxel is in
				glm::vec3 chunk_position = Chunk::GetBlockChunkPosition(voxel_position);
				Chunk* chunk = GetChunk(chunk_position);
				if (chunk == nullptr) continue;

				// If current voxel isnt void then store voxel to intersected
				glm::vec3 chunk_voxel_position = Chunk::GetBlockLocalPosition(voxel_position);
				if (!chunk->IsVoid(chunk_voxel_position) && !(Block::HasProperty(chunk->At(chunk_voxel_position).id, BlockProperty_DisableCollision))) {
					intersected.push_back(voxel_position);
				}
			}
		}
	}

	return intersected;
}
bool World::WillIntersect(const AABB& aabb, const glm::vec3& voxel) {
	// Get the min and max of effected area
	glm::vec3 min_voxel = Chunk::GetBlockPosition(aabb.min);
	glm::vec3 max_voxel = Chunk::GetBlockPosition(aabb.max);
	if (min_voxel.x <= voxel.x && min_voxel.y <= voxel.y && min_voxel.z <= voxel.z &&
		max_voxel.x >= voxel.x && max_voxel.y >= voxel.y && max_voxel.z >= voxel.z)
		return true;
	return false;
}
bool World::ResolveDynamicAABB(const AABB& aabb, glm::vec3& velocity, glm::vec3& normal) {
	
	// Find min and max of where collisions can occure
	glm::vec3 search_area_min = Chunk::GetBlockPosition(aabb.min) - 2.0f;
	glm::vec3 search_area_max = Chunk::GetBlockPosition(aabb.max) + 2.0f;

	// Search the search area for any occupied voxels
	std::vector<glm::vec3> potential_collisions;
	for (int y = (int)search_area_min.y; y < (int)search_area_max.y; y++) {
		for (int z = (int)search_area_min.z; z < (int)search_area_max.z; z++) {
			for (int x = (int)search_area_min.x; x < (int)search_area_max.x; x++) {
				glm::vec3 voxel = glm::vec3(x,y,z);
				if (!IsVoid(voxel) && !Block::HasProperty(GetVoxel(voxel).id, BlockProperty_DisableCollision)) potential_collisions.push_back(voxel + 0.5f); // +0.5 to center the voxel
			}
		}
	}
	if (potential_collisions.empty()) return false;

	// Check if potential collisions are collisions
	std::vector<std::pair<float, glm::vec3>> collisions;
	for (glm::vec3& voxel : potential_collisions) {
		CollisionResultData collision_result = DynamicAABBIntersection(aabb, velocity, AABB{ voxel, glm::vec3(1.0f) });
		if (collision_result.hit) collisions.push_back({ collision_result.dist, voxel });
	}

	// Sort collisions by distance
	std::sort(collisions.begin(), collisions.end(), [](const std::pair<float, glm::vec3>& left, const std::pair<float, glm::vec3>& right) {
		return left.first < right.first;
	});

	// Resolve collisions
	for (std::pair<float, glm::vec3>& collision : collisions) {
		glm::vec3 voxel = collision.second;
		CollisionResultData collision_result = DynamicAABBIntersection(aabb, velocity, AABB{ voxel, glm::vec3(1.0f) });
		glm::vec3 abs_velocity = { std::abs(velocity.x), std::abs(velocity.y), std::abs(velocity.z) };
		velocity += collision_result.normal * (abs_velocity * (1.0f - collision_result.dist) + 0.001f);
		normal += collision_result.normal;
	}
	normal = glm::normalize(normal);

	return !collisions.empty();
}

bool World::IsVoid(const glm::vec3& position) {
	// Get chunk
	glm::vec3 chunk_position = Chunk::GetBlockChunkPosition(position);
	Chunk* chunk = GetChunk(chunk_position);
	if (chunk == nullptr) return true;

	glm::vec3 local_voxel_position = Chunk::GetBlockLocalPosition(position);
	return chunk->IsVoid(local_voxel_position);
}
bool World::IsTransparent(const glm::vec3& position) {
	// Get chunk
	glm::vec3 chunk_position = Chunk::GetBlockChunkPosition(position);
	Chunk* chunk = GetChunk(chunk_position);
	if (chunk == nullptr) return true;

	glm::vec3 local_voxel_position = Chunk::GetBlockLocalPosition(position);
	return chunk->IsTransparent(local_voxel_position);
}

void World::RenderSkyBox(const Ref<Camera>& camera) {
	Ref<Shader>& sky_box_shader = Game::GetShader(ShaderType::SkyBox);
	sky_box_shader->Bind();
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	sky_box_shader->SetUniform("u_SkyColor", m_SkyColor);
	sky_box_shader->SetUniform("u_SkyHorizonColor", m_SkyHorizonColor);
	sky_box_shader->SetUniform("u_ViewProjection", camera->view_projection);
	sky_box_shader->SetUniform("u_CameraPosition", camera->position);

	Ref<VertexArray> skybox_mesh = Game::GetMesh(MeshType::SkyBox);
	skybox_mesh->Bind();
	glDrawElements(GL_TRIANGLES, skybox_mesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	skybox_mesh->Unbind();

	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	sky_box_shader->Unbind();
}
void World::RenderWorld(const Ref<Camera>& camera) {

	// Render World
	Ref<Texture>& block_atlas = Game::GetTexture(TextureType::BlockAtlas);
	block_atlas->Bind();

	Ref<Shader>& world_shader = Game::GetShader(ShaderType::World);
	world_shader->Bind();

	world_shader->SetUniform("u_ViewProjection", camera->view_projection);
	world_shader->SetUniform("u_Texture", block_atlas);
	world_shader->SetUniform("u_CameraPos", camera->position);
	world_shader->SetUniform("u_SkyHorizonColor", m_SkyHorizonColor);
	world_shader->SetUniform("u_Brightness", m_Brightness);
	world_shader->SetUniform("u_FogDistance", m_FogDistance);

	glm::vec3 world_position = glm::vec3(0.5f) + m_LoadedCenter * (float)Chunk::ChunkLength;
	glm::mat4 world = glm::translate(glm::mat4(1.0f), world_position); 

	glm::vec2 world_offset = {
		m_LoadedCenter.x - m_LoadedRadius,
		m_LoadedCenter.z - m_LoadedRadius
	};

	for (int z = 0; z < m_LoadedRadius * 2 + 1; z++) {
		for (int x = 0; x < m_LoadedRadius * 2 + 1; x++) {
			glm::vec3 position = { (x - m_LoadedRadius) * Chunk::ChunkLength, 0, (z - m_LoadedRadius) * Chunk::ChunkLength };
			glm::mat4 model = glm::translate(world, position);
			world_shader->SetUniform("u_Model", model);

			//Ref<Mesh<BlockVertex>> mesh = m_ChunkMeshes[x + z * m_LoadedWidth];
			Ref<Mesh<BlockVertex>> mesh = m_ChunkMeshes[glm::vec2(x, z) + world_offset];
			if (mesh == nullptr) continue;
			Ref<VertexArray> vao = mesh->GetVertexArray();
			if (vao == nullptr) continue;
			
			vao->Bind();
			glDrawElements(GL_TRIANGLES, vao->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
			vao->Unbind();
		}
	}

	world_shader->Unbind();
	block_atlas->Unbind();

	RenderEntities(camera);
	RenderParticals(camera);
}
void World::RenderEntities(const Ref<Camera>& camera)
{
	// Render entities in nearby chunks
	for (ItemEntity& item : m_Entities) {
		//float dist = glm::length(item.position - m_LoadedCenter * (float)Chunk::ChunkLength) / Chunk::ChunkLength;
		//if (dist >= m_EntityRenderDist) continue;
		item.Render(camera, m_Brightness); 
	}
}
void World::RenderParticals(const Ref<Camera>& camera) {
	for (Partical& partical : m_Particals) {
		partical.Render(camera, m_Brightness);
	}
}

void World::CalculateLightLevels(const glm::vec2& chunk_position) {
	Chunk* chunk = GetChunk({ chunk_position.x, 0, chunk_position.y });
	if (chunk == nullptr) return;

	// Find all light emitters in the chunk
	std::vector<glm::vec3> light_emitters;
	for (int y = 0; y < Chunk::ChunkHeight; y++) {
		for (int z = 0; z < Chunk::ChunkLength; z++) {
			for (int x = 0; x < Chunk::ChunkLength; x++) {
				Block& block = chunk->At({ x,y,z });
				if (Block::HasProperty(block.id, BlockProperty_LightEmitting)) {
					light_emitters.push_back(chunk->ToWorld({ x,y,z }));
				}
			}
		}
	}

	for (glm::vec3& light_emitter : light_emitters) {
		PropagateLight(light_emitter);
	}
}
void World::PropagateLight(const glm::vec3& position) {
	glm::vec3 start_chunk_pos = Chunk::GetBlockChunkPosition(position);
	Chunk* start_chunk = GetChunk(start_chunk_pos);
	if (start_chunk == nullptr) return;
	start_chunk->SetLightLevel(Chunk::GetBlockLocalPosition(position), MaxLightLevel);

	std::vector<glm::vec3> light_queue;
	light_queue.push_back(position);
	
	auto Propagate = [&](uint8_t light_level, const glm::vec3& other) {

		glm::vec3 chunk_pos = Chunk::GetBlockChunkPosition(other);
		Chunk* chunk = GetChunk(chunk_pos);
		if (chunk == nullptr) return;

		glm::vec3 local_pos = Chunk::GetBlockLocalPosition(other);

		uint8_t block_id = chunk->At(local_pos).id;
		if (!Block::HasProperty(block_id, BlockProperty_Transparent) && block_id != 0) return;
		if (chunk->GetLightLevel(local_pos) + 2 > light_level) return;

		chunk->SetLightLevel(local_pos, light_level - 1);
		light_queue.push_back(other);
	};

	while (!light_queue.empty()) {
		glm::vec3 light_position = light_queue[light_queue.size() - 1];
		light_queue.pop_back();

		glm::vec3 chunk_pos = Chunk::GetBlockChunkPosition(light_position);
		Chunk* chunk = GetChunk(chunk_pos);
		if (chunk == nullptr) continue;

		uint8_t light_level = chunk->GetLightLevel(Chunk::GetBlockLocalPosition(light_position));
		Propagate(light_level, light_position + glm::vec3( 1,  0,  0));
		Propagate(light_level, light_position + glm::vec3(-1,  0,  0));
		Propagate(light_level, light_position + glm::vec3( 0,  1,  0));
		Propagate(light_level, light_position + glm::vec3( 0, -1,  0));
		Propagate(light_level, light_position + glm::vec3( 0,  0,  1));
		Propagate(light_level, light_position + glm::vec3( 0,  0, -1));

	}

}

void World::ShowImGui() {
	ImGui::Text("Chunk Count: %d", m_Chunks.size());
	ImGui::Text("Loaded Radius: %d", (int)m_LoadedRadius);
	ImGui::Text("Loaded Center: %s", VEC3_STR(m_LoadedCenter).c_str());
	ImGui::InputFloat3("Sky Color", &m_SkyColor.r);
	ImGui::InputFloat3("Sky Horizon Color", &m_SkyHorizonColor.r);
	ImGui::DragFloat("Brightness", &m_Brightness, 0.01f, 0.0f, 1.0f);
	ImGui::Text("Entity Count: %d", m_Entities.size());
}

void World::Update(float delta_time, const Ref<Player>& player) {
	
}
void World::PhysicsUpdate(float delta_time, const Ref<Player>& player) {
	UpdateEntities(delta_time, player);
	UpdateParticals(delta_time, player);
}
void World::UpdateEntities(float delta_time, const Ref<Player>& player) {
	for (int i = 0; i < m_Entities.size(); i++) {
		ItemEntity& item = m_Entities[i];
		bool destory_item = item.Update(delta_time, player);

		// Delete item entity if needed
		if (destory_item) {
			m_Entities.erase(m_Entities.begin() + i);
			i--;
			continue;
		}

		// Check for/resolve collisions
		glm::vec3 collision_normal = glm::vec3(0.0f);
		AABB collider = { item.position, item.size };
		ResolveDynamicAABB(collider, item.velocity, collision_normal);
		item.position += item.velocity;

		// Check if on ground
		AABB ground_check;
		ground_check.position = item.position;
		ground_check.position.y -= item.size.y - 0.05f;
		ground_check.size = { item.size.x, 0.1f, item.size.z };
		ground_check.CalculateMinMax();
		item.on_ground = !AABBIntersectedVoxels(ground_check).empty();

		if (item.item.count == Item::StackSize) continue;

		// Check if close to other entities
		for (int j = 0; j < m_Entities.size(); j++) {
			if (i == j) continue;

			ItemEntity& other = m_Entities[j];
			if (other.item.id != item.item.id) continue;
			if (other.item.count == Item::StackSize) continue;

			float dist = glm::length(item.position - other.position);
			if (dist <= 0.75f) {
				// Merge stacks
				uint8_t item_count = item.item.count;
				if (other.item.count + item_count > Item::StackSize) item_count = Item::StackSize - other.item.count;
				other.item.count += item_count;
				item.item.count -= item_count;

				// Delete other stack
				if (item.item.count == 0) {
					m_Entities.erase(m_Entities.begin() + i);
					if (j > i) {
						j--;
						j = std::max(j, 0);
					}
					i--;
					i = std::max(i, 0);
				}
			}
		}
	}
}
void World::UpdateParticals(float delta_time, const Ref<Player>& player) {
	for (int i = 0; i < m_Particals.size(); i++) {
		Partical& partical = m_Particals[i];
		bool destory_item = partical.Update(delta_time, player);

		// Delete partical if needed
		if (destory_item) {
			m_Particals.erase(m_Particals.begin() + i);
			i--;
			continue;
		}

		// Check for/resolve collisions
		glm::vec3 collision_normal = glm::vec3(0.0f);
		glm::vec3 size = glm::vec3(0.1f);
		AABB collider = { partical.position, size };
		ResolveDynamicAABB(collider, partical.velocity, collision_normal);
		partical.position += partical.velocity;

		// Check if on ground
		AABB ground_check;
		ground_check.position = partical.position;
		ground_check.position.y -= size.y - 0.05f;
		ground_check.size = { size.x, 0.1f, size.z };
		ground_check.CalculateMinMax();
		partical.on_ground = !AABBIntersectedVoxels(ground_check).empty();
	}
}

void World::CreateChunk(const glm::vec2& position) {
	if (!m_Chunks.count(position)) m_Chunks.insert({ position, Chunk({ position.x - m_LoadedRadius + m_LoadedCenter.x, 0, position.y - m_LoadedRadius + m_LoadedCenter.z }, m_Seed) });
	else m_Chunks[position] = Chunk({ position.x - m_LoadedRadius + m_LoadedCenter.x, 0, position.y - m_LoadedRadius + m_LoadedCenter.z }, m_Seed);
	m_ChunkMeshes[position]->GetVertexArray() = nullptr;
}

ChunkMesher::ChunkMesher(Chunk* chunk, World* world): m_Chunk(chunk), m_World(world), m_VertexOffset(0) {}
ChunkMesher::~ChunkMesher() { }

Ref<Mesh<BlockVertex>> ChunkMesher::CreateMesh()
{
	m_Vertices.reserve(Chunk::ChunkDataSize);
	m_Indices.reserve(Chunk::ChunkDataSize);
	m_VertexOffset = 0;

	// Calculate vertices and indices
	for (int y = 0; y < Chunk::ChunkHeight; y++) {
		for (int z = 0; z < Chunk::ChunkLength; z++) {
			for (int x = 0; x < Chunk::ChunkLength; x++) {

				glm::vec3 position = { x,y,z };
				if (m_Chunk->IsVoid(position)) continue;

				uint8_t block_id = m_Chunk->At({ x,y,z }).id;
				uint8_t actual_id = (block_id & 0b00111111);

				if (Block::HasProperty(actual_id, BlockProperty_CrossMesh)) {
					MeshCrossMesh(position, block_id);
					continue;
				}

				const TextureIDs& textures = Block::GetTextureIDs(actual_id);
				MeshFace(position, { 0,  0,  1 }, textures.front, block_id, Block::FrontVertices);
				MeshFace(position, { 0,  0, -1 }, textures.back, block_id, Block::BackVertices);
				MeshFace(position, { 1,  0,  0 }, textures.left, block_id, Block::LeftVertices);
				MeshFace(position, { -1,  0,  0 }, textures.right, block_id, Block::RightVertices);
				MeshFace(position, { 0,  1,  0 }, textures.top, block_id, Block::TopVertices);
				MeshFace(position, { 0, -1,  0 }, textures.bottom, block_id, Block::BottomVertices);
			}
		}
	}

	// Create mesh
	Ref<Mesh<BlockVertex>> mesh = CreateRef<Mesh<BlockVertex>>(m_Vertices, m_Indices, Block::Layout);
	return mesh;
}

bool ChunkMesher::IsVoid(const glm::vec3& position) {
	if (m_Chunk->IsValid(position)) return m_Chunk->IsTransparent(position);
	glm::vec3 world_pos = {
		m_Chunk->GetPosition().x * Chunk::ChunkLength + position.x,
		position.y,
		m_Chunk->GetPosition().z * Chunk::ChunkLength + position.z,
	};
	glm::vec3 other_chunk_pos = Chunk::GetBlockChunkPosition(world_pos);
	Chunk* other_chunk = m_World->GetChunk(other_chunk_pos);
	if (!other_chunk) return true;
	return other_chunk->IsTransparent(Chunk::GetBlockLocalPosition(position));
}
void ChunkMesher::MeshFace(const glm::vec3& position, const glm::vec3& face_dir, uint32_t id, uint8_t block_id, const BlockVertex* data) {

	uint8_t orientation = (block_id & Block::OrientationMask) >> 6;
	int axis_count = Block::GetAxisCount(block_id);

	glm::vec3 adj_face_dir = Block::OrientVector(face_dir, axis_count, orientation);
	glm::vec3 other_pos = position + adj_face_dir;

	uint8_t light_level = 16;

	// Check if adjacent block is in the curent chunk
	if (!m_Chunk->IsValid(other_pos) && other_pos.y >= 0 && other_pos.y < Chunk::ChunkHeight) {
		
		// Get chunk the adjacent block is in
		glm::vec3 other_world_pos = {
			m_Chunk->GetPosition().x * Chunk::ChunkLength + other_pos.x,
			other_pos.y,
			m_Chunk->GetPosition().z * Chunk::ChunkLength + other_pos.z,
		};
		glm::vec3 other_chunk_pos = Chunk::GetBlockChunkPosition(other_world_pos);
		Chunk* other_chunk = m_World->GetChunk(other_chunk_pos);
		
		// Don't mesh face if at the edge of the loaded world
		if (!other_chunk) return;

		// Find adjacent block position in the other chunk
		if      (adj_face_dir.x > 0) other_pos.x = 0;
		else if (adj_face_dir.x < 0) other_pos.x = Chunk::ChunkLength - 1;
		else if (adj_face_dir.z > 0) other_pos.z = 0;
		else if (adj_face_dir.z < 0) other_pos.z = Chunk::ChunkLength - 1;
		
		light_level = other_chunk->GetLightLevel(other_pos);

		// Don't mesh face if both blocks are glass
		if (block_id == BlockID_Glass && Block::HasProperty(block_id, BlockProperty_Transparent) && !other_chunk->IsVoid(other_pos)) return;
		// Mesh face if adjacent block is not transparent
		else if (!other_chunk->IsTransparent(other_pos)) return;
	}
	// Don't mesh face if both blocks are glass
	else if (block_id == BlockID_Glass && Block::HasProperty(block_id, BlockProperty_Transparent) && !m_Chunk->IsVoid(other_pos)) return;
	// Mesh face if adjacent block is not void
	else if (!m_Chunk->IsTransparent(other_pos)) return;

	if (light_level == 16) light_level = m_Chunk->GetLightLevel(other_pos);

	// Add face data to mesh
	for (int i = 0; i < Block::FaceVertexCount; i++) {
		BlockVertex vertex = data[i];
		vertex.position = BlockVertex::PackPosition(Block::OrientVector(BlockVertex::UnpackPosition(vertex.position) - 0.5f, axis_count, orientation) + 0.5f);

		uint32_t data = 0;
		data |= vertex.data & (24 << 24); // Copy tex coords
		data |= Block::OrientVector(vertex.data & (7 << 24), axis_count, orientation); // orient normal

		// Calculate ambient occlusion
		glm::vec3 step = (BlockVertex::UnpackPosition(vertex.position) - 0.5f) * 2.0f;

		glm::vec3 left_voxel_offset = glm::vec3(0.0f);
		glm::vec3 right_voxel_offset = glm::vec3(0.0f);
		if (adj_face_dir.x != 0) {
			left_voxel_offset = glm::vec3(step.x, step.y, 0.0f);
			right_voxel_offset = glm::vec3(step.x, 0.0f, step.z);
		}
		else if (adj_face_dir.y != 0) {
			left_voxel_offset = glm::vec3(step.x, step.y, 0.0f);
			right_voxel_offset = glm::vec3(0.0f, step.y, step.z);
		}
		else if (adj_face_dir.z != 0) {
			left_voxel_offset = glm::vec3(step.x, 0.0f, step.z);
			right_voxel_offset = glm::vec3(0.0f, step.y, step.z);
		}
		uint32_t left_voxel   = !IsVoid(position + left_voxel_offset);
		uint32_t right_voxel  = !IsVoid(position + right_voxel_offset);
		uint32_t corner_voxel = !IsVoid(position + step);

		uint32_t ambient_occlusion = 0;
		if (left_voxel && right_voxel) ambient_occlusion = 0;
		else ambient_occlusion = (3 - (left_voxel + right_voxel + corner_voxel));
		data |= ambient_occlusion << 29;

		glm::vec3 vertex_position = BlockVertex::UnpackPosition(vertex.position);
		vertex_position += position;
		vertex.position = BlockVertex::PackPosition(vertex_position);

		data |= light_level << 20; // set light level
		vertex.data = data | id;
		m_Vertices.push_back(vertex);
	}
	for (int i = 0; i < Block::FaceIndexCount; i++) {
		m_Indices.push_back(Block::FaceIndices[i] + m_VertexOffset);
	}
	m_VertexOffset += Block::FaceVertexCount;
}
void ChunkMesher::MeshCrossMesh(const glm::vec3& position, uint8_t block_id) {
	const TextureIDs& texture_ids = Block::GetTextureIDs(block_id);
	for (int i = 0; i < 12; i++) {
		BlockVertex vertex = Block::CrossMeshVertices[i];
		
		glm::vec3 vertex_position = BlockVertex::UnpackPosition(vertex.position);
		vertex_position += position;
		vertex.position = BlockVertex::PackPosition(vertex_position);

		vertex.data |= texture_ids.front; // set id
		vertex.data |= m_Chunk->GetLightLevel(position) << 20; // set light level
		vertex.data |= 2 << 29; // ambient occlusion
		m_Vertices.push_back(vertex);
	}
	for (int i = 0; i < 24; i++) {
		m_Indices.push_back(Block::CrossMeshIndices[i] + m_VertexOffset);
	}
	m_VertexOffset += 12;
}

void WorldGenerator::GenerateChunk(Chunk* chunk, uint32_t seed) {

	std::vector<int> height_map;
	height_map.resize(Chunk::ChunkArea);

	for (int z = 0; z < Chunk::ChunkLength; z++) {
		for (int x = 0; x < Chunk::ChunkLength; x++) {
			GenerateColumn({ x,z }, height_map[x + z * Chunk::ChunkLength], chunk, seed);
		}
	}

	// Generate Ore Vains
	GenerateOreVains(chunk, height_map, 4, 15, 50, BlockID_CoalOre, seed); // Coal Ore
	GenerateOreVains(chunk, height_map, 4, 10, 45, BlockID_CopperOre, seed); // Copper Ore
	GenerateOreVains(chunk, height_map, 2, 8, 40, BlockID_IronOre, seed); // Iron Ore
	GenerateOreVains(chunk, height_map, 1, 5, 25, BlockID_IndiumOre, seed); // Indium Ore

	// Generate Foleage
	GenerateGrass(chunk, height_map, seed);
	GenerateTrees(chunk, height_map, seed);
}
void WorldGenerator::GenerateColumn(const glm::vec2& position, int& column_height, Chunk* chunk, uint32_t seed) {
	glm::vec3 voxel_position = {
		chunk->m_Position.x + ((float)position.x / (float)Chunk::ChunkLength),
		0.0f,
		chunk->m_Position.z + ((float)position.y / (float)Chunk::ChunkLength)
	};

	float surface_noise = NoiseGenerator::SampleFractalPerlinNoise2D(glm::vec2(voxel_position.x, voxel_position.z) * 0.0625f, 4, seed);
	surface_noise = (surface_noise + 1.0f) / 2.0f;
	column_height = std::max(std::min(surface_noise * (Chunk::ChunkHeight / 2.0f), (float)Chunk::ChunkHeight - 1), 0.0f) + 20;

	for (int y = 0; y <= column_height; y++) {
		voxel_position.y = ((float)y / (float)Chunk::ChunkLength);

		float cave_noise = NoiseGenerator::SampleFractalPerlinNoise3D(voxel_position, 2, seed);
		cave_noise = (cave_noise + 1.0f) / 2.0f;

		uint8_t block_id = 0;
		//bool is_border = (position.x == 0 || position.y == 0 || position.x == Chunk::ChunkLength - 1 || position.y == Chunk::ChunkLength - 1);

		if (y == 0)                          block_id = BlockID_Bedrock;
		else if (cave_noise > 0.70f)              block_id = BlockID_Air;
		else if (y == column_height - 1)          block_id = BlockID_Dirt;
		//else if (y == column_height && is_border) block_id = BlockID_Dirt; // Border Dirt
		else if (y == column_height)              block_id = BlockID_Grass;
		else                                      block_id = BlockID_Stone;

		Block& block = chunk->At({ position.x,y, position.y });
		block.id = block_id;
	}
}
void WorldGenerator::GenerateOreVains(Chunk* chunk, const std::vector<int>& height_map, int min, int max, int count, uint8_t ore_block_id, uint32_t seed) {
	uint32_t state = ((seed << (int)chunk->m_Position.x >> ore_block_id) ^ (seed >> (int)chunk->m_Position.z)) << ore_block_id;
	for (int i = 0; i < count; i++) {

		// Find random starting point
		glm::vec3 position = {
			(int)NoiseGenerator::RandomFloatRange(state, 0.0f, Chunk::ChunkLength - 1),
			0.0f,
			(int)NoiseGenerator::RandomFloatRange(state, 0.0f, Chunk::ChunkLength - 1)
		};
		position.y = (int)NoiseGenerator::RandomFloatRange(state, 1.0f, height_map[position.x + position.y * Chunk::ChunkLength] - 5);
		int ore_count = std::max((int)(state = NoiseGenerator::PCGHash(state)) % max, min);

		// Place ore_count number of ore blocks
		for (int j = 0; j < ore_count; j++) {
			uint8_t& block_id = chunk->At(position).id;
			if (block_id == BlockID_Stone) block_id = ore_block_id;

			int next = (state = NoiseGenerator::PCGHash(state)) % 6;
			glm::vec3 next_offset = glm::vec3(0.0f);
			if (next == 1) next_offset.x += 1;
			else if (next == 2) next_offset.x -= 1;
			else if (next == 3) next_offset.y += 1;
			else if (next == 4) next_offset.y -= 1;
			else if (next == 5) next_offset.z += 1;
			else if (next == 6) next_offset.z -= 1;

			// Go other direction if stone isnt there
			if (!chunk->IsValid(position + next_offset)) next_offset = -next_offset;
			else if (chunk->At(position + next_offset).id != BlockID_Stone) next_offset = -next_offset;
			position += next_offset;
		}
	}
}
void WorldGenerator::GenerateTrees(Chunk* chunk, const std::vector<int>& height_map, uint32_t seed) {

	float tree_density = 0.5f;
	float noise_scale = 4.0f;
	float tree_padding = 2.0f;

	uint32_t state = NoiseGenerator::State(chunk->m_Position, seed);

	std::vector<glm::vec3> tree_origins;

	for (int z = 0; z < Chunk::ChunkLength; z++) {
		for (int x = 0; x < Chunk::ChunkLength; x++) {

			// Don't place a tree if theres no grass
			int terrain_height = height_map[x + z * Chunk::ChunkLength];
			if (chunk->At({ x, terrain_height, z }).id != BlockID_Grass) continue;

			glm::vec2 noise_position = {
				chunk->m_Position.x + x / (float)Chunk::ChunkLength,
				chunk->m_Position.z + z / (float)Chunk::ChunkLength
			};
			float noise = NoiseGenerator::SamplePerlinNoise2D(noise_position * noise_scale, seed);

			if (noise <= 1.0f - tree_density) continue;
			state = NoiseGenerator::PCGHash((state << x) ^ (state >> z));
			if ((state % 100) / 100.0f >= 0.15f) continue;

			// Don't place a tree if one is already close by
			bool has_neighbor = false;
			glm::vec2 min_area = glm::vec2(x, z) - tree_padding;
			glm::vec2 max_area = glm::vec2(x, z) + tree_padding;
			for (glm::vec3& origin : tree_origins) {
				if (origin.x >= min_area.x && origin.x <= max_area.x &&
					origin.z >= min_area.y && origin.z <= max_area.y)
				{
					has_neighbor = true;
					break;
				}
			}
			if (has_neighbor) continue;

			glm::vec3 origin = { x, terrain_height + 1, z };
			tree_origins.push_back(origin);

			Structure tree = CreateTree(chunk, origin, seed);
			GenerateStructure(chunk, tree);
		}
	}
}
void WorldGenerator::GenerateGrass(Chunk* chunk, const std::vector<int>& height_map, uint32_t seed) {

	float noise_scale = 5.5f;
	float grass_density = 0.65f;

	uint32_t state = NoiseGenerator::State(chunk->m_Position, seed);

	for (int z = 0; z < Chunk::ChunkLength; z++) {
		for (int x = 0; x < Chunk::ChunkLength; x++) {

			// Don't place a grass if theres no grass
			int terrain_height = height_map[x + z * Chunk::ChunkLength];
			if (chunk->At({ x, terrain_height, z }).id != BlockID_Grass) continue;

			glm::vec2 noise_position = {
				chunk->m_Position.x + x / (float)Chunk::ChunkLength,
				chunk->m_Position.z + z / (float)Chunk::ChunkLength
			};
			float noise = NoiseGenerator::SamplePerlinNoise2D(noise_position * noise_scale, seed);

			if (noise <= 1.0f - grass_density) continue;
			state = NoiseGenerator::PCGHash((state << x) ^ (state >> z));
			float flower_noise = (state % 100) / 100.0f;

			uint8_t block_id = 0;
			if (flower_noise >= 0.80f && flower_noise < 0.90f) block_id = BlockID_PoppyFlower;
			else if (flower_noise >= 0.90f)                         block_id = BlockID_DandelionFlower;
			else                                                    block_id = BlockID_ShortGrass;

			chunk->At({ x, terrain_height + 1, z }).id = block_id;
		}
	}
}
void WorldGenerator::GenerateStructure(Chunk* chunk, const Structure& structure) {
	for (int i = 0; i < structure.positions.size(); i++) {
		glm::vec3 position = structure.positions[i];
		uint8_t id = structure.ids[i];

		if (chunk->IsValid(position)) {
			chunk->At(position).id = id;
			continue;
		}
	}
}
Structure WorldGenerator::CreateTree(Chunk* chunk, const glm::vec3& base_position, uint32_t seed) {

	uint32_t state = NoiseGenerator::State(chunk->ToWorld(base_position), seed);
	Structure tree;

	// Calculate tree trunk height
	state = NoiseGenerator::PCGHash(state);
	int tree_height = 4 + (state % 3);

	// Place dirt
	tree.positions.push_back(base_position - glm::vec3(0.0f, 1.0f, 0.0f));
	tree.ids.push_back(BlockID_Dirt);

	// Create tree trunk
	for (int i = 0; i < tree_height + 1; i++) {
		glm::vec3 position = base_position;
		position.y += i;
		tree.positions.push_back(position);
		tree.ids.push_back(BlockID_Log | 64); // or 64 to make log vertical
	}

	// Leave block data (there has got to be a better way to do this...)
	const uint8_t c_Leaves[] = {
		// Layer 1
		BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves,
		BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves,
		BlockID_Leaves, BlockID_Leaves, BlockID_Air,    BlockID_Leaves, BlockID_Leaves,
		BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves,
		BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves,

		// Layer 2
		BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves,
		BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves,
		BlockID_Leaves, BlockID_Leaves, BlockID_Air,    BlockID_Leaves, BlockID_Leaves,
		BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves,
		BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves,

		// Layer 3
		BlockID_Air, BlockID_Air,    BlockID_Air,    BlockID_Air,    BlockID_Air,
		BlockID_Air, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Air,
		BlockID_Air, BlockID_Leaves, BlockID_Air,    BlockID_Leaves, BlockID_Air,
		BlockID_Air, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Air,
		BlockID_Air, BlockID_Air,    BlockID_Air,    BlockID_Air,    BlockID_Air,

		// Layer 4
		BlockID_Air, BlockID_Air,    BlockID_Air,    BlockID_Air,    BlockID_Air,
		BlockID_Air, BlockID_Air,    BlockID_Leaves, BlockID_Air,    BlockID_Air,
		BlockID_Air, BlockID_Leaves, BlockID_Leaves, BlockID_Leaves, BlockID_Air,
		BlockID_Air, BlockID_Air,    BlockID_Leaves, BlockID_Air,    BlockID_Air,
		BlockID_Air, BlockID_Air,    BlockID_Air,    BlockID_Air,    BlockID_Air
	};

	// Create leaves
	for (int y = 0; y < 4; y++) {
		for (int z = 0; z < 5; z++) {
			for (int x = 0; x < 5; x++) {
				uint8_t id = c_Leaves[x + z * 5 + y * 25];
				if (id == BlockID_Air) continue;

				glm::vec3 position = base_position + glm::vec3(x - 2, y + tree_height - 3, z - 2);
				tree.positions.push_back(position);
				tree.ids.push_back(id);
			}
		}
	}

	return tree;
}
