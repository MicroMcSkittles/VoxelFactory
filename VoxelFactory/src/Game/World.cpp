#include "Game/World.h"
#include <algorithm>
#include <iostream>
#include <glad/glad.h>
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
	std::fill(m_Blocks.begin(), m_Blocks.begin() + ChunkArea * 16, Block{ 1 });
	for (int x = 0; x < ChunkLength; x++) {
		At({ x, 15, 0 }).id = 3;
		At({ x, 15, ChunkLength - 1 }).id = 3;
		At({ 0, 15, x }).id = 3;
		At({ ChunkLength - 1, 15, x }).id = 3;
	}

	uint32_t blockID = 1;
	for (int y = 0; y < ChunkHeight; y++) {
		if (y < 16) continue;
		for (int z = 0; z < ChunkLength; z++) {
			for (int x = 0; x < ChunkLength; x++) {
				if (x % 4 || y % 4 || z % 4) continue;
				At({ x,y,z }).id = blockID++;
				if (blockID > Block::BlockTextureIDs.size()) blockID = 1;
			}
		}
	}
}
Chunk::~Chunk() { }

Block& Chunk::At(const glm::vec3& position) {
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
		position.y + m_Position.y * ChunkHeight,
		position.z + m_Position.z * ChunkLength
	};
}
glm::vec3 Chunk::GetBlockChunkPosition(const glm::vec3& position) {
	return {
		floor(position.x / Chunk::ChunkLength),
		floor(position.y / Chunk::ChunkHeight),
		floor(position.z / Chunk::ChunkLength),
	};
}

World::World() : m_LoadedCenter({ 0,0,0 }), m_LoadedRadius(1) {
	m_LoadedDiameter = m_LoadedRadius * 2 + 1;
	m_LoadedArea = m_LoadedDiameter * m_LoadedDiameter;

	m_ChunkIndices.reserve(m_LoadedArea);
	m_Chunks.reserve(m_LoadedArea);
	m_ChunkMeshes.resize(m_LoadedArea);

	for (int z = -m_LoadedRadius; z <= m_LoadedRadius; z++) {
		for (int x = -m_LoadedRadius; x <= m_LoadedRadius; x++) {
			m_ChunkIndices.push_back((x + m_LoadedRadius) + (z + m_LoadedRadius) * m_LoadedDiameter);
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
	size_t index = (position.x - m_LoadedCenter.x + m_LoadedRadius);
	index += (position.z - m_LoadedCenter.x + m_LoadedRadius) * m_LoadedDiameter;
	if (index >= m_Chunks.size()) return nullptr;
	return &m_Chunks[m_ChunkIndices[index]];
}

void World::Render(const Ref<Camera>& camera) {
	m_Atlas->Bind();
	m_MainShader->Bind();

	m_MainShader->SetUniform("u_ViewProjection", camera->view_projection);
	m_MainShader->SetUniform("u_Texture", m_Atlas);

	glm::vec3 world_position = {
		m_LoadedCenter.x * Chunk::ChunkLength,
		m_LoadedCenter.y * Chunk::ChunkHeight,
		m_LoadedCenter.z * Chunk::ChunkLength,
	};
	glm::mat4 world = glm::mat4(1.0f);//= glm::translate(glm::mat4(1.0f), world_position);

	for (int z = 0; z < m_LoadedRadius * 2 + 1; z++) {
		for (int x = 0; x < m_LoadedRadius * 2 + 1; x++) {
			glm::vec3 position = { (x - m_LoadedRadius) * Chunk::ChunkLength, 0, (z - m_LoadedRadius) * Chunk::ChunkLength };
			glm::mat4 model = glm::translate(world, position);
			m_MainShader->SetUniform("u_Model", model);

			Ref<VertexArray>& vao = m_ChunkMeshes[x + z * m_LoadedDiameter];
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
		std::cout << "Moved Chunks: " << chunk_delta.x << ", " << chunk_delta.y << ", " << chunk_delta.z << std::endl;
		m_LoadedCenter = chunk_position;
		LoadChunks(chunk_delta);
	}
}
void World::LoadChunks(const glm::vec3& delta) {
	
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
	constexpr size_t stride = 5 * sizeof(float) + sizeof(uint32_t);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, stride, (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);
	vertex_array->GetVertexBuffer() = vertex_buffer;

	Ref<IndexBuffer> index_buffer = CreateRef<IndexBuffer>(m_Indices.data(), m_Indices.size() * sizeof(uint32_t));
	vertex_array->GetIndexBuffer() = index_buffer;

	vertex_array->Unbind();
	return vertex_array;
}
void ChunkMesher::MeshFace(const glm::vec3& position, const glm::vec3& face_dir, uint32_t id, const ChunkVertex* data) {
	
	glm::vec3 other_pos = position + face_dir;

	// Check if adjacent block is in the curent chunk
	if (!m_Chunk->IsValid(other_pos) && other_pos.y >= 0 && other_pos.y < Chunk::ChunkHeight) {

		// Get chunk the adjacent block is in
		glm::vec3 other_chunk_pos = Chunk::GetBlockChunkPosition(other_pos);
		Chunk* other_chunk = m_World->GetChunk(other_chunk_pos);

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
		vertex.position += position;
		vertex.id = id;
		m_Vertices.push_back(vertex);
	}
	for (int i = 0; i < c_FaceIndexCount; i++) {
		m_Indices.push_back(c_Indices[i] + m_VertexOffset);
	}
	m_VertexOffset += c_FaceVertexCount;
}













/*glm::vec3 other = position + face_dir;
	if (!m_Chunk->IsValid(other)) {
		other.x = (int)(Chunk::ChunkLength + other.x) % Chunk::ChunkLength;
		other.y = (int)(Chunk::ChunkHeight + other.y) % Chunk::ChunkHeight;
		other.z = (int)(Chunk::ChunkLength + other.z) % Chunk::ChunkLength;
		Chunk* chunk = m_World->GetChunk(m_Chunk->m_Position + face_dir);
		if (chunk != nullptr && !chunk->IsVoid(other)) return;
	}
	else if (!m_Chunk->IsVoid(other)) return;*/