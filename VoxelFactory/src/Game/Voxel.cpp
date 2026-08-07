#include "Game/Voxel.h"

#include <glad/glad.h>

#define TEX_COORD(x,y) y * 16 + x

Block Block::Invalid = Block{ Block::InvalidID };
std::vector<TextureIDs> Block::BlockTextureIDs = {
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

const BlockVertex Block::FrontVertices[] = {
	{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
	{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
	{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } }
};
const BlockVertex Block::BackVertices[] = {
	{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },
	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } },
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
	{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } },
	{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } }
};
const BlockVertex Block::LeftVertices[] = {
	{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
	{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
	{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
	{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
	{ {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
	{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }
};
const BlockVertex Block::RightVertices[] = {
	{ { -0.5f,  0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
	{ { -0.5f,  0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
	{ { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
	{ { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
	{ { -0.5f, -0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
	{ { -0.5f,  0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } }
};
const BlockVertex Block::TopVertices[] = {
	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
	{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
	{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
	{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
	{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } }
};
const BlockVertex Block::BottomVertices[] = {
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },
	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },
	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
	{ { -0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } }
};
const BlockVertex Block::Vertices[] = {
	// Front
	{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
	{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
	{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },

	// Back
	{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },
	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } },
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
	{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } },
	{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },

	// Left
	{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
	{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
	{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
	{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
	{ {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
	{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },

	// Right
	{ { -0.5f,  0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
	{ { -0.5f,  0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
	{ { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
	{ { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
	{ { -0.5f, -0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
	{ { -0.5f,  0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },

	// Top
	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
	{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
	{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
	{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
	{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },

	// Bottom
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },
	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },
	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
	{ { -0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } }
};
const uint32_t Block::FaceIndices[] = {
	0,  1,  2,
	3,  4,  5
};
const uint32_t Block::Indices[] = {
	0,  1,  2,  3,  4,  5,  // Front
	6,  7,  8,  9,  10, 11, // Back
	12, 13, 14, 15, 16, 17, // Left
	18, 19, 20, 21, 22, 23, // Right
	24, 25, 26, 27, 28, 29, // Top
	30, 31, 32, 33, 34, 35  // Bottom
};

const VertexLayout Block::Layout = { {
	{ GL_FLOAT, 3 }, // a_Pos
	{ GL_FLOAT, 3 }, // a_Normal
	{ GL_FLOAT, 2 }, // a_TexCoord
	{ GL_FLOAT, 1 }, // a_AmbientOcclution
	{ GL_UNSIGNED_INT, 1 }, // a_TextureID
} };

void Block::InitMesh() {
	std::vector<BlockVertex> vertices(Vertices, Vertices + 36);
	size_t offset = 0;
	for (int i = 0; i < FaceVertexCount; i++, offset++) vertices[offset].id = 0; // Front
	for (int i = 0; i < FaceVertexCount; i++, offset++) vertices[offset].id = 1; // Back
	for (int i = 0; i < FaceVertexCount; i++, offset++) vertices[offset].id = 2; // Left
	for (int i = 0; i < FaceVertexCount; i++, offset++) vertices[offset].id = 3; // Right
	for (int i = 0; i < FaceVertexCount; i++, offset++) vertices[offset].id = 4; // Top
	for (int i = 0; i < FaceVertexCount; i++, offset++) vertices[offset].id = 5; // Bottom

	Mesh = CreateRef<VertexArray>();
	Mesh->Bind();

	Ref<VertexBuffer> vertex_buffer = CreateRef<VertexBuffer>(vertices.data(), vertices.size() * sizeof(BlockVertex), Layout);
	Mesh->GetVertexBuffer() = vertex_buffer;

	Ref<IndexBuffer> index_buffer = CreateRef<IndexBuffer>(Indices, 36 * sizeof(uint32_t));
	Mesh->GetIndexBuffer() = index_buffer;

	Mesh->Unbind();
}

bool Block::HasOrientation(uint8_t id) {
	uint8_t actual_id = id & 0b00111111;
	if (actual_id == 5 || actual_id == 18 || actual_id == 19) return true;
	return false;
}
int Block::GetAxisCount(uint8_t id) {
	if (id == 5) return 3; // 3 possable orentations
	if (id == 18 || id == 19) return 4; // 4 possable orentations
	return 0;
}
uint8_t Block::CalculateOrientation(const glm::vec3& direction, uint8_t id) {
	glm::vec3 orientation = glm::vec3(0.0f);
	if (abs(direction.x) > abs(direction.y) && abs(direction.x) > abs(direction.z)) orientation = glm::vec3(direction.x, 0.0f, 0.0f);
	else if (abs(direction.z) > abs(direction.y)) orientation = glm::vec3(0.0f, 0.0f, direction.z);
	else orientation = glm::vec3(0.0f, direction.y, 0.0f);
	orientation = glm::normalize(orientation);

	int axis_count = GetAxisCount(id);

	if (axis_count == 3) {
		if (orientation.x != 0) return 0b00000000;
		if (orientation.y != 0) return 0b01000000;
		if (orientation.z != 0) return 0b10000000;
	}
	if (axis_count == 4) {
		if (orientation.x > 0) return 0b00000000;
		if (orientation.z > 0) return 0b01000000;
		if (orientation.x < 0) return 0b10000000;
		if (orientation.z < 0) return 0b11000000;
	}

	return 0;
}

glm::vec3 Block::OrientVector(const glm::vec3& direction, int axis_count, uint8_t orientation) {
	if (axis_count == 3) {
		if (orientation == 0) return glm::vec3(-direction.y, direction.x, direction.z);
		if (orientation == 1) return direction;
		if (orientation == 2) return glm::vec3(direction.x, -direction.z, direction.y);
	}
	if (axis_count == 4) {
		if (orientation == 0) return glm::vec3(direction.z, direction.y, -direction.x);
		if (orientation == 1) return direction;
		if (orientation == 3) return glm::vec3(-direction.x, direction.y, -direction.z);
		if (orientation == 2) return glm::vec3(-direction.z, direction.y, direction.x);
	}

	return direction;
}
