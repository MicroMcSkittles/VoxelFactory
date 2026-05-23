#include "Game/World.h"
#include "Core/Utils.h"

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

Chunk::Chunk(const glm::vec3& position) {
	m_Position = position;

	// TODO: actual generation

	m_Blocks.resize(ChunkDataSize, Block{ 0 });
	
	int dirt_height = 16 + m_Position.x + m_Position.z;
	std::fill(m_Blocks.begin(), m_Blocks.begin() + ChunkArea * (dirt_height - 1), Block{ 4 });
	std::fill(m_Blocks.begin() + ChunkArea * (dirt_height - 1), m_Blocks.begin() + ChunkArea * dirt_height, Block{ 1 });
	for (int x = 0; x < ChunkLength; x++) {
		At({ x, dirt_height - 1, 0 }).id = 3;
		At({ x, dirt_height - 1, ChunkLength - 1 }).id = 3;
		At({ 0, dirt_height - 1, x }).id = 3;
		At({ ChunkLength - 1, dirt_height - 1, x }).id = 3;
	}

	/*uint32_t blockID = 1;
	for (int y = 0; y < ChunkHeight; y++) {
		if (y < dirt_height) continue;
		for (int z = 0; z < ChunkLength; z++) {
			for (int x = 0; x < ChunkLength; x++) {
				if (x % 4 || y % 4 || z % 4) continue;
				At({ x,y,z }).id = blockID++;
				if (blockID > Block::BlockTextureIDs.size()) blockID = 1;
			}
		}
	}*/
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

World::World() : m_LoadedCenter({ 0,0,0 }), m_LoadedRadius(3) {
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
		m_ChunkMeshes[i] = ChunkMesher(&m_Chunks[i], this).Mesh();
	}

	m_MainShader = CreateRef<Shader>("assets/shaders/Main.vert", "assets/shaders/Main.frag");
	m_Atlas = CreateRef<Texture>("assets/textures/atlas.png");
}
World::~World() { }

Chunk* World::GetChunk(const glm::vec3& position) {
	// Check if chunk position is in bounds
	if (position.x < -m_LoadedRadius || position.x > m_LoadedRadius ||
		position.z < -m_LoadedRadius || position.z > m_LoadedRadius) return nullptr;

	// Get chunk index
	size_t index = position.x + m_LoadedRadius + (position.z + m_LoadedRadius) * m_LoadedWidth;
	if (index >= m_Chunks.size()) return nullptr;
	return &m_Chunks[index];
}
void World::RebuildChunk(const glm::vec3& position) {
	Chunk* chunk = GetChunk(position);
	if (chunk == nullptr) return;
	m_ChunkMeshes[position.x + m_LoadedRadius + (position.z + m_LoadedRadius) * m_LoadedWidth] = ChunkMesher(chunk, this).Mesh();
}

RayResultData World::RayAABBIntersection(const Ray& ray, const glm::vec3& aabb_min, const glm::vec3& aabb_max) {
	
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
	return { false };
}
RayResultData World::CastRay(const Ray& ray) {

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
		RayResultData aabb_result = RayAABBIntersection(ray, aabb_min, aabb_max);
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
	RayResultData result;
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
		else if (max_dist.z < max_dist.y) {
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

void World::Render(const Ref<Camera>& camera) {
	m_Atlas->Bind();
	m_MainShader->Bind();

	m_MainShader->SetUniform("u_ViewProjection", camera->view_projection);
	m_MainShader->SetUniform("u_Texture", m_Atlas);

	glm::vec3 world_position = glm::vec3(0.5f);
	glm::mat4 world = glm::translate(glm::mat4(1.0f), world_position); 

	for (int z = 0; z < m_LoadedRadius * 2 + 1; z++) {
		for (int x = 0; x < m_LoadedRadius * 2 + 1; x++) {
			glm::vec3 position = { (x - m_LoadedRadius) * Chunk::ChunkLength, 0, (z - m_LoadedRadius) * Chunk::ChunkLength };
			glm::mat4 model = glm::translate(world, position);
			m_MainShader->SetUniform("u_Model", model);

			Ref<VertexArray>& vao = m_ChunkMeshes[x + z * m_LoadedWidth];
			vao->Bind();
			glDrawElements(GL_TRIANGLES, vao->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
			vao->Unbind();
		}
	}

	m_MainShader->Unbind();
	m_Atlas->Unbind();
}
void World::Update(const glm::vec3& position) {
	glm::vec3 chunk_position = Chunk::GetBlockChunkPosition(position);
	glm::vec3 chunk_delta = chunk_position - m_LoadedCenter;
	if (chunk_delta.x != 0 || chunk_delta.y != 0 || chunk_delta.z != 0) {
		m_LoadedCenter = chunk_position;
		LoadChunks({ chunk_delta.x, chunk_delta.y });
	}
}
void World::ShowImGui() {
	ImGui::Text("Chunk Count: %d", m_Chunks.size());
	ImGui::Text("Loaded Radius: %d", m_LoadedRadius);
}

void World::LoadChunks(const glm::vec2& delta) {
	
}

ChunkMesher::ChunkMesher(Chunk* chunk, World* world): m_Chunk(chunk), m_World(world), m_VertexOffset(0) {}
ChunkMesher::~ChunkMesher() { }

Ref<VertexArray> ChunkMesher::Mesh() {

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
				MeshFace(position, {  0,  0,  1 }, textures.front,  c_FrontVertices);
				MeshFace(position, {  0,  0, -1 }, textures.back,   c_BackVertices);
				MeshFace(position, {  1,  0,  0 }, textures.left,   c_LeftVertices);
				MeshFace(position, { -1,  0,  0 }, textures.right,  c_RightVertices);
				MeshFace(position, {  0,  1,  0 }, textures.top,    c_TopVertices);
				MeshFace(position, {  0, -1,  0 }, textures.bottom, c_BottomVertices);
			}
		}
	}

	// Create vertex array
	Ref<VertexArray> vertex_array = CreateRef<VertexArray>();
	vertex_array->Bind();

	Ref<VertexBuffer> vertex_buffer = CreateRef<VertexBuffer>(m_Vertices.data(), m_Vertices.size() * sizeof(ChunkVertex));
	vertex_buffer->Bind();
	constexpr size_t stride = 9 * sizeof(float) + sizeof(uint32_t);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, stride, (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(4);
	vertex_array->GetVertexBuffer() = vertex_buffer;

	Ref<IndexBuffer> index_buffer = CreateRef<IndexBuffer>(m_Indices.data(), m_Indices.size() * sizeof(uint32_t));
	vertex_array->GetIndexBuffer() = index_buffer;

	vertex_array->Unbind();
	return vertex_array;
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