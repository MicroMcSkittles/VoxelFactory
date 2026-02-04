#include "Game/World.h"
#include <algorithm>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#define TEX_COORD(x,y) y * 16 + x

Block Block::Invalid = Block{ Block::InvalidID };
std::vector<Block::TextureIDs> Block::BlockTextureIDs = {
	{ TEX_COORD(3,0), TEX_COORD(0,0), TEX_COORD(2,0) }, // Grass
	{ TEX_COORD(2,0) }, // Dirt
	{ TEX_COORD(1,0) }, // Stone
	{ TEX_COORD(4,1), TEX_COORD(5, 1), TEX_COORD(5,1) }, // Log
	{ TEX_COORD(4,0) }, // Planks
	{ TEX_COORD(3,2), TEX_COORD(4, 0), TEX_COORD(4,0) }, // Bookshelf
	{ TEX_COORD(7,0) }, // Bricks
	{ TEX_COORD(0,1) }, // Cobble stone
	{ TEX_COORD(1,1) }, // Gravel
	{ TEX_COORD(2,1) }, // Sand
	{ TEX_COORD(0,2) }, // Iron ore
	{ TEX_COORD(2,2) }, // Coal ore
	{ TEX_COORD(3,3) }, // Copper ore
	{ TEX_COORD(12,2), TEX_COORD(14,2), TEX_COORD(13,2), TEX_COORD(13,2), TEX_COORD(14,3), TEX_COORD(14,3) }, // Furnace
	{ TEX_COORD(11,3), TEX_COORD(11,3), TEX_COORD(12,3), TEX_COORD(11,3), TEX_COORD(11,2), TEX_COORD(10,4) }, // Work bench
};

Chunk::Chunk(const glm::vec3& position) {
	m_Position = position;
	m_Blocks.resize(ChunkDataSize, Block{ 0 });

	// TODO: actual generation
	uint32_t blockID = 1;
	for (int y = 0; y < ChunkHeight; y++) {
		for (int z = 0; z < ChunkLength; z++) {
			for (int x = 0; x < ChunkWidth; x++) {
				if (x % 4 || y % 4 || z % 4) continue;
				At({ x,y,z }).id = blockID++;
				if (blockID > Block::BlockTextureIDs.size()) blockID = 1;
			}
		}
	}

	// Divid chunks
	/*for (int x = 0; x < ChunkWidth; x++) {
		At({ x, ChunkHeight - 1, 0 }).id = 3;
		At({ x, ChunkHeight - 1, ChunkLength - 1 }).id = 3;
		At({ 0, ChunkHeight - 1, x }).id = 3;
		At({ ChunkLength - 1, ChunkHeight - 1, x }).id = 3;
	}*/

}
Chunk::~Chunk() { }
Block& Chunk::At(const glm::vec3& position) {
	size_t index = position.x + position.z * ChunkWidth + position.y * ChunkLength * ChunkWidth;
	if (index >= ChunkDataSize) return Block::Invalid;
	return m_Blocks[index];
}
bool Chunk::IsVoid(const glm::vec3& position) {
	Block& block = At(position);
	return block.id == Block::InvalidID || block.id == 0;
}
bool Chunk::IsValid(const glm::vec3& position) {
	if (position.x < 0 || position.x >= ChunkWidth)  return false;
	if (position.y < 0 || position.y >= ChunkHeight) return false;
	if (position.z < 0 || position.z >= ChunkLength) return false;
	return true;
}
glm::vec3 Chunk::ToWorld(const glm::vec3& position) {
	return {
		position.x + m_Position.x * ChunkWidth,
		position.y + m_Position.y * ChunkHeight,
		position.z + m_Position.z * ChunkLength
	};
}

// TODO: impl the y value

World::World() : m_LoadedCenter({ 0,0,0 }), m_LoadedRadius(5) {
	m_LoadedChunks.reserve((m_LoadedRadius * 2 + 1) * (m_LoadedRadius * 2 + 1));
	m_ChunkMeshes.resize((m_LoadedRadius * 2 + 1) * (m_LoadedRadius * 2 + 1));

	for (int z = -m_LoadedRadius; z <= m_LoadedRadius; z++) {
		for (int x = -m_LoadedRadius; x <= m_LoadedRadius; x++) {
			m_LoadedChunks.push_back(Chunk{ { x, 0, z } });
		}
	}

	for (size_t i = 0; i < m_LoadedChunks.size(); i++) {
		m_ChunkMeshes[i] = ChunkMesher(&m_LoadedChunks[i], this).Mesh();
	}

	m_MainShader = CreateRef<Shader>("assets/shaders/Main.vert", "assets/shaders/Main.frag");
	m_Atlas = CreateRef<Texture>("assets/textures/atlas.png");
}
World::~World() { }

//Block& World::At(const glm::vec3& position) {
//	
//	glm::vec3 chunk_coord = {
//		(position.x / Chunk::ChunkWidth) - m_LoadedCenter.x + m_LoadedRadius,
//		//floor(position.y / Chunk::ChunkHeight) - m_LoadedCenter.y + m_LoadedRadius,
//		position.y / Chunk::ChunkHeight,
//		(position.z / Chunk::ChunkLength) - m_LoadedCenter.z + m_LoadedRadius
//	};
//	size_t chunk_index = floor(chunk_coord.x) + floor(chunk_coord.z) * (m_LoadedRadius * 2 + 1);
//	if (chunk_index >= m_LoadedChunks.size()) return Block::Invalid;
//
//	glm::vec3 subchunk_coord = {
//		(chunk_coord.x - floor(chunk_coord.x)) * Chunk::ChunkWidth,
//		(chunk_coord.y - floor(chunk_coord.y)) * Chunk::ChunkHeight,
//		(chunk_coord.z - floor(chunk_coord.z)) * Chunk::ChunkLength
//	};
//
//	return m_LoadedChunks[chunk_index].At(subchunk_coord);
//}
//bool World::IsVoid(const glm::vec3& position) {
//	Block& block = At(position);
//	return block.id == Block::InvalidID || block.id == 0;
//}
Chunk* World::GetChunk(const glm::vec3& position) {
	if (position.y != 0) return nullptr;
	size_t index = (position.x - m_LoadedCenter.x + m_LoadedRadius) + (position.z - m_LoadedCenter.x + m_LoadedRadius) * (m_LoadedRadius * 2 + 1);
	if (index >= m_LoadedChunks.size()) return nullptr;
	return &m_LoadedChunks[index];
}

void World::Render(const Ref<Camera>& camera) {
	m_Atlas->Bind();
	m_MainShader->Bind();

	m_MainShader->SetUniform("u_ViewProjection", camera->view_projection);
	m_MainShader->SetUniform("u_Texture", m_Atlas);

	for (int z = 0; z < m_LoadedRadius * 2 + 1; z++) {
		for (int x = 0; x < m_LoadedRadius * 2 + 1; x++) {
			glm::vec3 position = { (x - m_LoadedRadius) * Chunk::ChunkWidth, 0, (z - m_LoadedRadius) * Chunk::ChunkLength };
			glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
			m_MainShader->SetUniform("u_Model", model);

			Ref<VertexArray>& vao = m_ChunkMeshes[x + z * (m_LoadedRadius * 2 + 1)];
			vao->Bind();
			glDrawElements(GL_TRIANGLES, vao->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
			vao->Unbind();
		}
	}

	m_MainShader->Unbind();
	m_Atlas->Unbind();
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
			for (int x = 0; x < Chunk::ChunkWidth; x++) {

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
void ChunkMesher::MeshFace(const glm::vec3& position, const glm::vec3& face, uint32_t id, const ChunkVertex* data) {
	glm::vec3 other = position + face;
	if (!m_Chunk->IsValid(other)) {
		other.x = (int)(Chunk::ChunkWidth + other.x) % Chunk::ChunkWidth;
		other.y = (int)(Chunk::ChunkHeight + other.y) % Chunk::ChunkHeight;
		other.z = (int)(Chunk::ChunkLength + other.z) % Chunk::ChunkLength;
		Chunk* chunk = m_World->GetChunk(m_Chunk->m_Position + face);
		if (chunk != nullptr && !chunk->IsVoid(other)) return;
	}
	else if (!m_Chunk->IsVoid(other)) return;
	
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