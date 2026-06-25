#include "Game/World.h"
#include "Core/Utils.h"
#include "Core/ImGuiUtils.h"
#include "Game/Noise.h"

#include <algorithm>
#include <iostream>
#include <limits>

#include <imgui.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define TEX_COORD(x,y) y * 16 + x

Block Block::Invalid = Block{ Block::InvalidID };
std::vector<Block::TextureIDs> Block::BlockTextureIDs = {
	{ TEX_COORD(3,0), TEX_COORD(0,0), TEX_COORD(2,0) }, // Grass
	{ TEX_COORD(4,4), TEX_COORD(2,4), TEX_COORD(2,0) }, // Snowy Grass
	{ TEX_COORD(2,0) }, // Dirt
	{ TEX_COORD(1,0) }, // Stone
	{ TEX_COORD(4,1), TEX_COORD(5, 1), TEX_COORD(5,1) }, // Log
	{ TEX_COORD(4,0) }, // Planks
	{ TEX_COORD(3,2), TEX_COORD(4, 0), TEX_COORD(4,0) }, // Bookshelf
	{ TEX_COORD(7,0) }, // Bricks
	{ TEX_COORD(6,3) }, // Stone bricks
	{ TEX_COORD(6,0) }, // Polished stone
	{ TEX_COORD(0,1) }, // Cobble stone
	{ TEX_COORD(1,1) }, // Gravel
	{ TEX_COORD(2,1) }, // Sand
	{ TEX_COORD(0,2) }, // Iron ore
	{ TEX_COORD(2,2) }, // Coal ore
	{ TEX_COORD(3,3) }, // Copper ore
	{ TEX_COORD(1,3) }, // Glass
	{ TEX_COORD(12,2), TEX_COORD(14,2), TEX_COORD(13,2), TEX_COORD(13,2), TEX_COORD(14,3), TEX_COORD(14,3) }, // Furnace
	{ TEX_COORD(11,3), TEX_COORD(11,3), TEX_COORD(12,3), TEX_COORD(11,3), TEX_COORD(11,2), TEX_COORD(10,4) }, // Work bench
};

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

Chunk::Chunk(const glm::vec3& position) {
	m_Position = position;

	m_Blocks.resize(ChunkDataSize, Block{ 0 });
	
	for (int z = 0; z < ChunkLength; z++) {
		for (int x = 0; x < ChunkLength; x++) {
			float noise = NoiseGenerator::SamplePerlinNoise({ x,z }, { position.x, position.z }, 2, 6942067) * 0.5;
			noise += NoiseGenerator::SamplePerlinNoise({ x,z }, { position.x, position.z }, 4, 6942067) * 0.25;
			noise += NoiseGenerator::SamplePerlinNoise({ x,z }, { position.x, position.z }, 8, 6942067) * 0.125;
			noise += NoiseGenerator::SamplePerlinNoise({ x,z }, { position.x, position.z }, 16, 6942067) * 0.0625;
			noise = (noise + 1.0f) / 2.0f;
			int height = std::max(std::min(noise * (ChunkHeight / 8.0f), (float)ChunkHeight), 0.0f);
			for (int y = 0; y < height; y++) {
				At({ x,y + 20,z }).id = 4; // Stone
			}
			At({ x,height + 19, z }).id = 3; // Dirt
			At({ x,height + 20, z }).id = 1; // Grass
		}
	}
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
bool Chunk::IsVoid(const glm::vec3& position) {
	Block& block = At(position);
	return block.id == Block::InvalidID || block.id == 0;
}
bool Chunk::IsValid(const glm::vec3& position) {
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

World::World() : m_LoadedCenter({ 0,0,0 }), m_LoadedRadius(6) {
	m_LoadedWidth = m_LoadedRadius * 2 + 1;
	m_LoadedArea = m_LoadedWidth * m_LoadedWidth;

	m_Chunks.reserve(m_LoadedArea);
	m_ChunkMeshes.resize(m_LoadedArea);

	for (int z = -m_LoadedRadius; z <= m_LoadedRadius; z++) {
		for (int x = -m_LoadedRadius; x <= m_LoadedRadius; x++) {
			m_Chunks.push_back(Chunk{ { x, 0, z } });
		}
	}
	for (size_t i = 0; i < m_Chunks.size(); i++) {
		m_ChunkMeshes[i] = ChunkMesher(&m_Chunks[i], this).CreateMesh();
		m_ChunkMeshes[i]->CreateVertexArray();
	}

	m_MainShader = CreateRef<Shader>("assets/shaders/Main.vert", "assets/shaders/Main.frag");
	m_Atlas = CreateRef<Texture>("assets/textures/atlas.png");
}
World::~World() { }

void World::SetVoxel(const glm::vec3& position, uint8_t new_id) {
	glm::vec3 local_position = Chunk::GetBlockLocalPosition(position);
	glm::vec3 chunk_position = Chunk::GetBlockChunkPosition(position);
	Chunk* chunk = GetChunk(chunk_position);
	if (chunk == nullptr) return;
	chunk->At(local_position).id = new_id;

	// Rebuild affected chunks
	RebuildChunk(chunk_position);
	if (local_position.x == 0) RebuildChunk({ chunk_position.x - 1, 0, chunk_position.z });
	else if (local_position.x == Chunk::ChunkLength - 1)RebuildChunk({ chunk_position.x + 1, 0, chunk_position.z });
	if (local_position.z == 0) RebuildChunk({ chunk_position.x, 0, chunk_position.z - 1 });
	else if (local_position.z == Chunk::ChunkLength - 1) RebuildChunk({ chunk_position.x, 0, chunk_position.z + 1 });
}

Chunk* World::GetChunk(const glm::vec3& position) {
	// Check if chunk position is in bounds
	if (position.x - m_LoadedCenter.x < -m_LoadedRadius || position.x - m_LoadedCenter.x > m_LoadedRadius ||
		position.z - m_LoadedCenter.z < -m_LoadedRadius || position.z - m_LoadedCenter.z > m_LoadedRadius) return nullptr;

	// Get chunk index
	size_t index = position.x - m_LoadedCenter.x + m_LoadedRadius + (position.z - m_LoadedCenter.z + m_LoadedRadius) * m_LoadedWidth;
	if (index >= m_Chunks.size()) return nullptr;
	return &m_Chunks[index];
}
void World::RebuildChunk(const glm::vec3& position) {
	Chunk* chunk = GetChunk(position);
	if (chunk == nullptr) return;
	int index = position.x - m_LoadedCenter.x + m_LoadedRadius + (position.z - m_LoadedCenter.z + m_LoadedRadius) * m_LoadedWidth;
	m_ChunkMeshes[index] = ChunkMesher(chunk, this).CreateMesh();
	m_ChunkMeshes[index]->CreateVertexArray();
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
				if (!chunk->IsVoid(chunk_voxel_position)) {
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
				if (!IsVoid(voxel)) potential_collisions.push_back(voxel + 0.5f); // +0.5 to center the voxel
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

void World::Render(const Ref<Camera>& camera) {
	m_Atlas->Bind();
	m_MainShader->Bind();

	m_MainShader->SetUniform("u_ViewProjection", camera->view_projection);
	m_MainShader->SetUniform("u_Texture", m_Atlas);

	glm::vec3 world_position = glm::vec3(0.5f) + m_LoadedCenter * (float)Chunk::ChunkLength;
	glm::mat4 world = glm::translate(glm::mat4(1.0f), world_position); 

	for (int z = 0; z < m_LoadedRadius * 2 + 1; z++) {
		for (int x = 0; x < m_LoadedRadius * 2 + 1; x++) {
			glm::vec3 position = { (x - m_LoadedRadius) * Chunk::ChunkLength, 0, (z - m_LoadedRadius) * Chunk::ChunkLength };
			glm::mat4 model = glm::translate(world, position);
			m_MainShader->SetUniform("u_Model", model);

			Ref<VertexArray>& vao = m_ChunkMeshes[x + z * m_LoadedWidth]->GetVertexArray();
			if (vao == nullptr) continue;
			vao->Bind();
			glDrawElements(GL_TRIANGLES, vao->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
			vao->Unbind();
		}
	}

	m_MainShader->Unbind();
	m_Atlas->Unbind();
}
void World::ShowImGui() {
	ImGui::Text("Chunk Count: %d", m_Chunks.size());
	ImGui::Text("Loaded Radius: %d", m_LoadedRadius);
	ImGui::Text("Loaded Center: %s", VEC3_STR(m_LoadedCenter).c_str());
	ImGuiImage("Atlas", m_Atlas, { 0.0f, 150.0f });
}

void World::Update(const glm::vec3& position) {
	glm::vec3 chunk_position = Chunk::GetBlockChunkPosition(position);
	glm::vec3 chunk_delta = chunk_position - m_LoadedCenter;
	if (chunk_delta.x != 0 || chunk_delta.y != 0 || chunk_delta.z != 0) {
		m_LoadedCenter = chunk_position;
		MoveLoadedCenter({ chunk_delta.x, chunk_delta.z });
	}
}
void World::MoveLoadedCenter(const glm::vec2& delta) {
	// Shift existing chunks to new location
	int shift_count = 0;
	shift_count += (int)delta.x;
	shift_count += m_LoadedWidth * (int)delta.y;
	std::rotate(m_Chunks.begin(), ((shift_count > 0) ? m_Chunks.begin() : m_Chunks.end()) + shift_count, m_Chunks.end());
	std::rotate(m_ChunkMeshes.begin(), ((shift_count > 0) ? m_ChunkMeshes.begin() : m_ChunkMeshes.end()) + shift_count, m_ChunkMeshes.end());
	
	std::vector<glm::vec2> new_chunks;
	std::vector<glm::vec2> chunks_to_rebuild;

	// Find the chunks that need to be created
	int column = (delta.x < 0) ? 0 : m_LoadedWidth - 1;
	int row = (delta.y < 0) ? 0 : m_LoadedWidth - 1;
	if (delta.x != 0) {
		for (int i = 0; i < m_LoadedWidth; i++) {
			if (column == i && delta.y != 0) continue;
			new_chunks.push_back({ column, i });
		}
	}
	if (delta.y != 0) {
		for (int i = 0; i < m_LoadedWidth; i++) {
			if (row == i && delta.x != 0) continue;
			new_chunks.push_back({ i, row });
		}
	}
	if (delta.x != 0 && delta.y != 0) {
		new_chunks.push_back({ column, row });
	}

	// Find existing chunks that need to be rebuilt
	column -= delta.x;
	row -= delta.y;
	for (int i = 1; i < m_LoadedWidth - 1; i++) {
		if (delta.x != 0) chunks_to_rebuild.push_back({ column, i });
		if (delta.y != 0) chunks_to_rebuild.push_back({ i, row });
	}

	// Create new chunks
	for (auto& chunk : new_chunks) {
		CreateChunk(chunk);
	}

	// Build chunks
	for (auto& chunk : new_chunks) {
		int index = (int)chunk.x + m_LoadedWidth * (int)chunk.y;
		m_ChunkMeshes[index] = ChunkMesher(&m_Chunks[index], this).CreateMesh();
		m_ChunkMeshes[index]->CreateVertexArray();
	}
	for (auto& chunk : chunks_to_rebuild) {
		int index = (int)chunk.x + m_LoadedWidth * (int)chunk.y;
		m_ChunkMeshes[index] = ChunkMesher(&m_Chunks[index], this).CreateMesh();
		m_ChunkMeshes[index]->CreateVertexArray();
	}
}
void World::CreateChunk(const glm::vec2& position) {
	int index = position.x + m_LoadedWidth * position.y;
	m_Chunks[index] = Chunk({ position.x - m_LoadedRadius + m_LoadedCenter.x, 0, position.y - m_LoadedRadius + m_LoadedCenter.z});
}

ChunkMesher::ChunkMesher(Chunk* chunk, World* world): m_Chunk(chunk), m_World(world), m_VertexOffset(0) {}
ChunkMesher::~ChunkMesher() { }

Ref<Mesh<ChunkVertex>> ChunkMesher::CreateMesh()
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

				Block::TextureIDs& textures = Block::BlockTextureIDs[m_Chunk->At({ x,y,z }).id - 1];
				MeshFace(position, { 0,  0,  1 }, textures.front, c_FrontVertices);
				MeshFace(position, { 0,  0, -1 }, textures.back, c_BackVertices);
				MeshFace(position, { 1,  0,  0 }, textures.left, c_LeftVertices);
				MeshFace(position, { -1,  0,  0 }, textures.right, c_RightVertices);
				MeshFace(position, { 0,  1,  0 }, textures.top, c_TopVertices);
				MeshFace(position, { 0, -1,  0 }, textures.bottom, c_BottomVertices);
			}
		}
	}

	// Create mesh
	VertexLayout vertex_layout = { {
		{ GL_FLOAT, 3 }, // a_Pos
		{ GL_FLOAT, 3 }, // a_Normal
		{ GL_FLOAT, 2 }, // a_TexCoord
		{ GL_FLOAT, 1 }, // a_AmbientOcclution
		{ GL_UNSIGNED_INT, 1 }, // a_TextureID
	} };
	Ref<Mesh<ChunkVertex>> mesh = CreateRef<Mesh<ChunkVertex>>(m_Vertices, m_Indices, vertex_layout);
	return mesh;
}

bool ChunkMesher::IsVoid(const glm::vec3& position) {
	if (m_Chunk->IsValid(position)) return m_Chunk->IsVoid(position);
	glm::vec3 world_pos = {
		m_Chunk->GetPosition().x * Chunk::ChunkLength + position.x,
		position.y,
		m_Chunk->GetPosition().z * Chunk::ChunkLength + position.z,
	};
	glm::vec3 other_chunk_pos = Chunk::GetBlockChunkPosition(world_pos);
	Chunk* other_chunk = m_World->GetChunk(other_chunk_pos);
	if (!other_chunk) return true;
	return other_chunk->IsVoid(Chunk::GetBlockLocalPosition(position));
}
void ChunkMesher::MeshFace(const glm::vec3& position, const glm::vec3& face_dir, uint32_t id, const ChunkVertex* data) {
	
	glm::vec3 other_pos = position + face_dir;

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
		if      (face_dir.x > 0) other_pos.x = 0;
		else if (face_dir.x < 0) other_pos.x = Chunk::ChunkLength - 1;
		else if (face_dir.z > 0) other_pos.z = 0;
		else if (face_dir.z < 0) other_pos.z = Chunk::ChunkLength - 1;
		
		// Mesh face if adjacent block is not void
		if (!other_chunk->IsVoid(other_pos)) return;
	}
	// Mesh face if adjacent block is not void
	else if (!m_Chunk->IsVoid(other_pos)) return;

	// Add face data to mesh
	for (int i = 0; i < c_FaceVertexCount; i++) {
		ChunkVertex vertex = data[i];
		// Calculate ambient occlusion
		glm::vec3 step = vertex.position * 2.0f;

		glm::vec3 left_voxel_offset = glm::vec3(0.0f);
		glm::vec3 right_voxel_offset = glm::vec3(0.0f);
		if (face_dir.x != 0) {
			left_voxel_offset = glm::vec3(step.x, step.y, 0.0f);
			right_voxel_offset = glm::vec3(step.x, 0.0f, step.z);
		}
		else if (face_dir.y != 0) {
			left_voxel_offset = glm::vec3(step.x, step.y, 0.0f);
			right_voxel_offset = glm::vec3(0.0f, step.y, step.z);
		}
		else if (face_dir.z != 0) {
			left_voxel_offset = glm::vec3(step.x, 0.0f, step.z);
			right_voxel_offset = glm::vec3(0.0f, step.y, step.z);
		}
		uint32_t left_voxel   = !IsVoid(position + left_voxel_offset);
		uint32_t right_voxel  = !IsVoid(position + right_voxel_offset);
		uint32_t corner_voxel = !IsVoid(position + step);

		if (left_voxel && right_voxel) vertex.ambient_occlusion = 0;
		else vertex.ambient_occlusion = 3 - (left_voxel + right_voxel + corner_voxel);

		vertex.position += position;
		vertex.id = id;
		m_Vertices.push_back(vertex);
	}
	for (int i = 0; i < c_FaceIndexCount; i++) {
		m_Indices.push_back(c_Indices[i] + m_VertexOffset);
	}
	m_VertexOffset += c_FaceVertexCount;
}

