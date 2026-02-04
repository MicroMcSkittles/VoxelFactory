#include "Game/World.h"
#include <algorithm>
#include <glad/glad.h>

#define TEX_COORD(x,y) y * 16 + x

Block Block::Invalid = Block{ Block::InvalidID };
std::vector<Block::TextureIDs> Block::BlockTextureIDs = {
	{ TEX_COORD(3,0), TEX_COORD(0,0), TEX_COORD(2,0) }, // Grass
	{ TEX_COORD(2,0) }, // Dirt
	{ TEX_COORD(1,0) }, // Stone
};

Chunk::Chunk() {
	m_Position = glm::vec3(0.0f);
	m_Blocks.resize(ChunkDataSize, Block{ 0 });

	// TODO: actual generation
	uint32_t blockID = 1;
	for (int y = 0; y < ChunkHeight; y++) {
		if      (y == 15)           blockID = 1;
		else if (y < 15 && y >= 13) blockID = 2;
		else                        blockID = 3;
		for (int z = 0; z < ChunkLength; z++) {
			for (int x = 0; x < ChunkWidth; x++) {
				At({ x,y,z }).id = blockID;
			}
		}
	}
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

ChunkMesher::ChunkMesher(const Ref<Chunk>& chunk): m_Chunk(chunk), m_VertexOffset(0) { }
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
	if (m_Chunk->IsValid(other) && !m_Chunk->IsVoid(other)) return;
	
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